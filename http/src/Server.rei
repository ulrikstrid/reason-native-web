type route_callback('context) = (~request: Request.t, 'context) => Lwt.t(Response.t);

/**
[start http_port https_port cert priv_key context route_callback] starts the server.
If you provide [cert] and [priv_key] it will automatically start a https server with support fort HTTP/2

[context] can be anything, it will get passed to your [route_callback] on every new request.

{2 Simple usage:}
{[
Http.Server.start(~context=(), (~request as _, ()) => {
    Http.Response.Ok.make();
})
|> Lwt_main.run;
]}
*/
let start: (
    ~http_port: int=?,
    ~https_port: int=?,
    ~cert: string=?,
    ~priv_key: string=?,
    ~context: 'context,
    route_callback('context)
) => Lwt.t(unit);

