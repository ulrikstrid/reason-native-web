open HttpImpl;

module Ok = {
  let make = (~httpImpl, ~extra_headers=?, reqd) => {
    let headers =
      switch (extra_headers) {
      | Some(h) => [("content-length", "2"), ...h]
      | None => [("content-length", "2")]
      };

    HttpImpl.make_response(~status=`OK, ~headers, "ok");
  };
};

module Text = {
  let make = (~httpImpl, ~extra_headers=?, ~text, reqd) => {
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

    HttpImpl.make_response(~status=`OK, ~headers, text);
  };
};

module Json = {
  let make = (~httpImpl, ~json, reqd) => {
    let content_length = json |> String.length |> string_of_int;
    let headers = [
      ("content-type", "application/json"),
      ("content-length", content_length),
    ];

    HttpImpl.make_response(~status=`OK, ~headers, json);
  };
};

module Html = {
  let make = (~httpImpl, ~markup, reqd) => {
    let content_length = markup |> String.length |> string_of_int;
    let headers = [
      ("content-type", "text/html"),
      ("content-length", content_length),
    ];

    HttpImpl.make_response(~status=`OK, ~headers, markup);
  };
};

module Redirect = {
  let make = (~httpImpl, ~extra_headers=?, ~code=303, ~targetPath, reqd) => {
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

    HttpImpl.make_response(~status=`Code(code), ~headers, targetPath);
  };
};

module Unauthorized = {
  let make = (~httpImpl, message, reqd) => {
    let headers = [
      ("content-length", String.length(message) |> string_of_int),
    ];

    HttpImpl.make_response(~status=`Unauthorized, ~headers, message);
  };
};

module NotFound = {
  let make = (~httpImpl, ~message="Not found", reqd) => {
    let headers = [
      ("content-length", String.length(message) |> string_of_int),
    ];

    HttpImpl.make_response(~status=`Not_found, ~headers, message);
  };
};
