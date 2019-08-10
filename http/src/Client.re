open Lwt.Infix;

type response = {
  status: Httpaf.Status.t,
  body: string,
  headers: list((string, string)),
};

let error_handler = _ => assert(false);

let read_response = (~notify_finished, response, response_body) => {
  switch (response) {
  | {Httpaf.Response.status: `OK, headers, _} =>
    let content_length =
      Httpaf.Headers.get(headers, "content-length")
      |> CCOpt.flat_map(CCInt.of_string)
      |> CCOpt.get_or(~default=2048);

    let body_buffer = Buffer.create(content_length);

    let rec on_read = (bs, ~off, ~len) => {
      let request_data_bytes = Bytes.create(len);
      Bigstringaf.blit_to_bytes(
        bs,
        ~src_off=0,
        request_data_bytes,
        ~dst_off=off,
        ~len,
      );
      Buffer.add_bytes(body_buffer, request_data_bytes);
      Httpaf.Body.schedule_read(response_body, ~on_read, ~on_eof);
    }
    and on_eof = () => {
      Lwt.wakeup_later(
        notify_finished,
        Ok({
          status: `OK,
          body: Buffer.contents(body_buffer),
          headers: Httpaf.Headers.to_list(headers),
        }),
      );
    };

    Httpaf.Body.schedule_read(response_body, ~on_read, ~on_eof);
  | {headers, status, _} =>
    Format.fprintf(
      Format.err_formatter,
      "%a\n%!",
      Httpaf.Response.pp_hum,
      response,
    );
    let content_length =
      Httpaf.Headers.get(headers, "content-length")
      |> CCOpt.flat_map(CCInt.of_string)
      |> CCOpt.get_or(~default=2048);

    let body_buffer = Buffer.create(content_length);

    let rec on_read = (bs, ~off, ~len) => {
      let request_data_bytes = Bytes.create(len);
      Bigstringaf.blit_to_bytes(
        bs,
        ~src_off=0,
        request_data_bytes,
        ~dst_off=off,
        ~len,
      );
      Buffer.add_bytes(body_buffer, request_data_bytes);
      Httpaf.Body.schedule_read(response_body, ~on_read, ~on_eof);
    }
    and on_eof = () => {
      Lwt.wakeup_later(
        notify_finished,
        Error(`Msg(Httpaf.Status.to_string(status))),
      );
    };

    Httpaf.Body.schedule_read(response_body, ~on_read, ~on_eof);
  };
};

let fetch = (~meth=`GET, ~body=?, ~headers=[], url) => {
  let uri = Uri.of_string(url);

  let url_host = Uri.host_with_default(~default="", uri);
  let url_scheme =
    Uri.scheme(uri)
    |> (
      fun
      | Some(scheme) => scheme
      | None => "https"
    );
  let url_port =
    Uri.port(uri)
    |> (
      fun
      | Some(port) => string_of_int(port)
      | None when url_scheme == "https" => "443"
      | None => "80"
    );
  let url_path = Uri.path(uri);

  Lwt_unix.getaddrinfo(url_host, url_port, [Unix.(AI_FAMILY(PF_INET))])
  >>= (
    addresses => {
      let socket = Lwt_unix.socket(Unix.PF_INET, Unix.SOCK_STREAM, 0);

      Lwt_unix.connect(socket, List.hd(addresses).Unix.ai_addr)
      >>= (
        () => {
          let (finished, notify_finished) = Lwt.wait();

          let response_handler = read_response(~notify_finished);

          let headers =
            switch (body) {
            | Some(body) => [
                ("Content-Length", string_of_int(String.length(body))),
                ...headers,
              ]
            | None => headers
            };

          let headers =
            Httpaf.Headers.of_list([
              ("Connection", "close"),
              ("Host", url_host),
              ...headers,
            ]);

          let client =
            Lwt_ssl.embed_uninitialized_socket(
              socket,
              Ssl.create_context(Ssl.TLSv1_2, Ssl.Client_context),
            );

          let () =
            Ssl.set_client_SNI_hostname(
              Lwt_ssl.ssl_socket_of_uninitialized_socket(client),
              url_host,
            );

          Lwt_ssl.ssl_perform_handshake(client)
          >>= (
            client =>
              Httpaf_lwt_unix.Client.SSL.create_connection(
                ~client,
                Lwt_ssl.get_fd(client),
              )
              >>= (
                connection => {
                  let request_body =
                    Httpaf_lwt_unix.Client.SSL.request(
                      connection,
                      ~error_handler,
                      ~response_handler,
                      Httpaf.Request.create(~headers, meth, url_path),
                    );

                  switch (body) {
                  | Some(body) =>
                    Httpaf.Body.write_string(request_body, body)
                  | None => ()
                  };

                  Httpaf.Body.close_writer(request_body);
                  finished;
                }
              )
          );
        }
      );
    }
  );
};

let get = (~port=443, ~host, ~path, ~headers=[], ()) => {
  Lwt_unix.getaddrinfo(
    host,
    CCInt.to_string(port),
    [Unix.(AI_FAMILY(PF_INET))],
  )
  >>= (
    addresses => {
      let socket = Lwt_unix.socket(Unix.PF_INET, Unix.SOCK_STREAM, 0);
      Lwt_unix.connect(socket, CCList.hd(addresses).Unix.ai_addr)
      >>= (
        () => {
          let (finished, notify_finished) = Lwt.wait();
          let response_handler = read_response(~notify_finished);
          let headers = Httpaf.Headers.of_list([("host", host), ...headers]);

          let client =
            Lwt_ssl.embed_uninitialized_socket(
              socket,
              Ssl.create_context(Ssl.TLSv1_2, Ssl.Client_context),
            );

          let () =
            Ssl.set_client_SNI_hostname(
              Lwt_ssl.ssl_socket_of_uninitialized_socket(client),
              host,
            );

          Lwt_ssl.ssl_perform_handshake(client)
          >>= (
            client =>
              Httpaf_lwt_unix.Client.SSL.create_connection(~client, socket)
          )
          >>= (
            connection => {
              let request_body =
                Httpaf_lwt_unix.Client.SSL.request(
                  connection,
                  ~error_handler,
                  ~response_handler,
                  Httpaf.Request.create(~headers, `GET, path),
                );
              Httpaf.Body.close_writer(request_body);
              finished;
            }
          );
        }
      );
    }
  );
};
