Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Info));
Logs.set_reporter(Logs_fmt.reporter());

let root_handler = (_request, _context) => Http.Response.Ok.make();

let greet_handler = (greeting, _request, _context) => {
  Http.Response.Text.make(greeting);
};

let routes =
  Routes.(
    Routes.Infix.(
      with_method([
        (`GET, root_handler <$ s("")),
        (`GET, greet_handler <$> s("greet") *> str),
      ])
    )
  );

let routes_callback = (~request: Http.Request.t, context) => {
  Routes.match_with_method(~target=request.target, ~meth=request.meth, routes)
  |> (
    fun
    | Some(res) => res(request, context)
    | None => Http.Response.NotFound.make()
  );
};

Http.Server.start(~context=(), routes_callback) |> Lwt_main.run;
