type t = {
  status: Status.t,
  headers: list((string, string)),
  body: string,
};

let make = (~status=`OK, ~headers=[], body) => {status, headers, body};

module Ok = {
  let make = (~extra_headers=?, ()) => {
    let headers =
      switch (extra_headers) {
      | Some(h) => [("content-length", "2"), ...h]
      | None => [("content-length", "2")]
      };

    make(~status=`OK, ~headers, "ok") |> Lwt.return;
  };
};

module Text = {
  let make = (~extra_headers=?, text) => {
    let headers =
      switch (extra_headers) {
      | Some(h) => [
          ("content-length", CCString.length(text) |> CCInt.to_string),
          ...h,
        ]
      | None => [
          ("content-length", CCString.length(text) |> CCInt.to_string),
        ]
      };

    make(~status=`OK, ~headers, text) |> Lwt.return;
  };
};

module Json = {
  let make = (~extra_headers=[], json) => {
    let content_length = json |> String.length |> string_of_int;
    let headers = [
      ("content-type", "application/json"),
      ("content-length", content_length),
      ...extra_headers
    ];

    make(~status=`OK, ~headers, json) |> Lwt.return;
  };
};

module Html = {
  let make = (~extra_headers=[], markup) => {
    let content_length = markup |> String.length |> string_of_int;
    let headers = [
      ("content-type", "text/html"),
      ("content-length", content_length),
      ...extra_headers
    ];

    make(~status=`OK, ~headers, markup) |> Lwt.return;
  };
};

module Redirect = {
  let make = (~extra_headers=?, ~code=303, targetPath) => {
    let content_length = targetPath |> String.length |> string_of_int;

    let constantHeaders = [
      ("content-length", content_length),
      ("location", targetPath),
    ];

    let headers =
      switch (extra_headers) {
      | Some(h) => CCList.concat([constantHeaders, h])
      | None => constantHeaders
      };

    make(~status=`Code(code), ~headers, targetPath) |> Lwt.return;
  };
};

module Unauthorized = {
  let make = (~extra_headers=[], message) => {
    let headers = [
      ("content-length", String.length(message) |> string_of_int),
      ...extra_headers
    ];

    make(~status=`Unauthorized, ~headers, message) |> Lwt.return;
  };
};

module NotFound = {
  let make = (~extra_headers=[], ~message="Not found", ()) => {
    let headers = [
      ("content-length", String.length(message) |> string_of_int),
      ...extra_headers
    ];

    make(~status=`Not_found, ~headers, message) |> Lwt.return;
  };
};
