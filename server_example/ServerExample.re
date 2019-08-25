Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Info));
Logs.set_reporter(Logs_fmt.reporter());

Http.Server.start(~context=(), (~request as _, ()) => Http.Response.Ok.make())
|> Lwt_main.run;
