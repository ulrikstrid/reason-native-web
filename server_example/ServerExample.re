Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Info));
Logs.set_reporter(Logs_fmt.reporter());

let routes = {
  Routes.(
    Routes.Infix.(
      with_method([
        (`GET, Http.Response.Ok.make() <$ s("")),
        (`GET, Http.Response.Text.make <$> s("greet") *> str),
      ])
    )
  );
};

Http.Server.start(~context=(), (~request, ()) => {
  Routes.match_with_method(~target=request.target, ~meth=request.meth, routes)
  |> (
    fun
    | Some(res) => res
    | None => Http.Response.NotFound.make()
  )
})
|> Lwt_main.run;
