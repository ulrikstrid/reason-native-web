open HttpImpl;

module Ok = {
  let make = (~httpImpl, ~extra_headers=?, reqd) => {
    let headers =
      (
        switch (extra_headers) {
        | Some(h) => [("content-length", "2"), ...h]
        | None => [("content-length", "2")]
        }
      )
      |> httpImpl.headers_of_list;
    httpImpl.respond_with_string(
      reqd,
      httpImpl.create_response(~headers, `OK),
      "ok",
    );
    ();
  };
};

module Text = {
  let make = (~httpImpl, ~extra_headers=?, ~text, reqd) => {
    let headers =
      (
        switch (extra_headers) {
        | Some(h) => [
            ("content-length", CCString.length(text) |> CCInt.to_string),
            ...h,
          ]
        | None => [
            ("content-length", CCString.length(text) |> CCInt.to_string),
          ]
        }
      )
      |> httpImpl.headers_of_list;
    httpImpl.respond_with_string(
      reqd,
      httpImpl.create_response(~headers, `OK),
      text,
    );
    ();
  };
};

module Json = {
  let make = (~httpImpl, ~json, reqd) => {
    let content_length = json |> String.length |> string_of_int;
    httpImpl.respond_with_string(
      reqd,
      httpImpl.create_response(
        ~headers=
          httpImpl.headers_of_list([
            ("content-type", "application/json"),
            ("content-length", content_length),
          ]),
        `OK,
      ),
      json,
    );
  };
};

module Html = {
  let make = (~httpImpl, ~markup, reqd) => {
    let content_length = markup |> String.length |> string_of_int;
    httpImpl.respond_with_string(
      reqd,
      httpImpl.create_response(
        ~headers=
          httpImpl.headers_of_list([
            ("content-type", "text/html"),
            ("content-length", content_length),
          ]),
        `OK,
      ),
      markup,
    );
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
      (
        switch (extra_headers) {
        | Some(h) => CCList.concat([constantHeaders, h])
        | None => constantHeaders
        }
      )
      |> httpImpl.headers_of_list;

    httpImpl.respond_with_string(
      reqd,
      httpImpl.create_response(~headers, `Code(code)),
      targetPath,
    );
  };
};

module Unauthorized = {
  let make = (~httpImpl, message, reqd) => {
    httpImpl.respond_with_string(
      reqd,
      httpImpl.create_response(
        ~headers=
          httpImpl.headers_of_list([
            ("content-length", String.length(message) |> string_of_int),
          ]),
        `Code(401),
      ),
      message,
    );
    ();
  };
};

module NotFound = {
  let make = (~httpImpl, ~message="Not found", reqd) => {
    httpImpl.respond_with_string(
      reqd,
      httpImpl.create_response(
        ~headers=
          httpImpl.headers_of_list([
            ("content-length", String.length(message) |> string_of_int),
          ]),
        `Code(404),
      ),
      message,
    );
    ();
  };
};
