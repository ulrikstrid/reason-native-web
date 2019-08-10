type t('meth, 'headers, 'status, 'response, 'reqd) = {
  target: string,
  meth: 'meth,
  get_header: string => option(string),
  create_response: (~headers: 'headers, 'status) => 'response,
  respond_with_string: ('reqd, 'response, string) => unit,
  headers_of_list: list((string, string)) => 'headers,
  read_body: 'reqd => Lwt.t(string),
};
