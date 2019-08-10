let error_handler = (_client_address, ~request as _=?, _error, start_response) => {
  open H2;
  let response_body = start_response(Headers.empty);
  Body.close_writer(response_body);
};

let route_handler: ('a, 'b, Unix.sockaddr, H2.Reqd.t) => unit =
  (context, make_routes_callback, _client_address, request_descriptor) => {
    let start = Unix.gettimeofday();
    Lwt.async(() => {
      let H2.{Request.target, meth, headers, scheme: _} =
        H2.Reqd.request(request_descriptor);

      let content_length =
        H2.Headers.get(headers, "content-length")
        |> CCOpt.map_or(~default=128, int_of_string);

      let read_body =
        Body.read(
          ~content_length,
          ~get_request_body=H2.Reqd.request_body,
          ~schedule_read=H2.Body.schedule_read,
        );

      let create_response = (~headers, status) =>
        H2.Response.create(~headers, status);

      let httpImpl =
        HttpImpl.{
          target,
          meth,
          get_header: H2.Headers.get(headers),
          create_response,
          respond_with_string: H2.Reqd.respond_with_string,
          headers_of_list: H2.Headers.of_list,
          read_body,
        };

      make_routes_callback(~httpImpl, ~context, request_descriptor)
      |> Lwt.map(() => {
           let stop = Unix.gettimeofday();
           Logs.info(m =>
             m(
               "H2: %s request to %s finished in %fms",
               Method.to_string(meth),
               target,
               (stop -. start) *. 1000.,
             )
           );
         });
    });
  };
