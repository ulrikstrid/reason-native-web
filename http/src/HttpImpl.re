type t('meth, 'headers) = {
  target: string,
  meth: 'meth,
  get_header: string => option(string),
  read_body: unit => Lwt.t(string),
};

type response('status) = {
  status: 'status,
  headers: list((string, string)),
  body: string,
};

let make_response = (~status=`Ok, ~headers, body) => {status, headers, body};
