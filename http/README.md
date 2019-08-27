# @reason-native-web/http

A tiny framework for developing HTTP stuff in reason native. Includes a Fetch-inspired client and a abstractions over Httpaf and H2.

## Documentation

https://ulrikstrid.github.io/reason-native-web/reason-http/index.html

_The documentation is not as fleshed out as it should be, the repos is under heavy development._

## Example

**Server:**

```ocaml
Http.Server.start(~context=(), (~request as _, ()) => {
    Http.Response.Ok.make();
})
|> Lwt_main.run;
```

Slightly larger example using ocaml-router: https://github.com/ulrikstrid/reason-native-web/blob/master/server_example/ServerExample.re

**Client:**

```ocaml
(* GET example *)

Http.Client.fetch("/hello-world");
```

```ocaml
(* POST example *)

Http.Client.fetch(
    ~meth=`POST,
    ~headers=[("Content-Type", "application/json")],
    ~body={|{"hello": "world"}|},
    "/hello-world",
)
```

## Thanks

A huge thanks goes out to @anmonteiro, some of the code is borrowed from his examples and then modified over time. And obviously nothing if this would have happened if it were not for his great work refining Httpaf and creating H2.
