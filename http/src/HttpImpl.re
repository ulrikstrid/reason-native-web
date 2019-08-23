type t = {
  target: string,
  meth: Method.t,
  get_header: string => option(string),
  read_body: unit => Lwt.t(string),
};

type response = {
  status: Status.t,
  headers: list((string, string)),
  body: string,
};

let make_response = (~status=`OK, ~headers, body) => {status, headers, body};
