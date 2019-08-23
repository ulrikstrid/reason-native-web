let error_handler = (_client_address, ~request as _=?, _error, start_response) => {
  let response_body = start_response(Httpaf.Headers.empty);
  Httpaf.Body.close_writer(response_body);
};

let route_handler:
  ('context, 'make_routes_callback, Unix.sockaddr, 'reqd) => unit =
  (context, make_routes_callback, _client_address, request_descriptor) => {
    let start = Unix.gettimeofday();
    Lwt.async(() => {
      let Httpaf.{Request.target, meth, headers, version: _} =
        Httpaf.Reqd.request(request_descriptor);

      let content_length =
        Httpaf.Headers.get(headers, "content-length")
        |> CCOpt.map_or(~default=128, int_of_string);

      let read_body = () =>
        Body.read(
          ~content_length,
          ~get_request_body=Httpaf.Reqd.request_body,
          ~schedule_read=Httpaf.Body.schedule_read,
          request_descriptor,
        );

      let create_response = (~headers, status) =>
        Httpaf.Response.create(
          ~headers,
          ~reason=?None,
          ~version=?None,
          status,
        );

      let httpImpl =
        HttpImpl.{
          target,
          meth,
          get_header: Httpaf.Headers.get(headers),
          read_body,
        };

      make_routes_callback(~httpImpl, context)
      |> Lwt.map((response: HttpImpl.response) => {
           let headers = Httpaf.Headers.of_list(response.headers);

           let () =
             Httpaf.Reqd.respond_with_string(
               request_descriptor,
               create_response(~headers, `Code(Status.to_code(response.status))),
               response.body,
             );

           let stop = Unix.gettimeofday();
           Logs.info(m =>
             m(
               "http: %s request to %s finished in %fms",
               Method.to_string(meth),
               target,
               (stop -. start) *. 1000.,
             )
           );
         });
    });
  };
