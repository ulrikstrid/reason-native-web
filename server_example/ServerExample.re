Http.Server.start(~context=(), (~request as _, ()) => Http.Response.Ok.make())
|> Lwt_main.run;
