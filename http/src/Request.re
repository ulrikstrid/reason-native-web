type t = {
    target: string,
    meth: Method.t,
    get_header: string => option(string),
    read_body: unit => Lwt.t(string),
};
