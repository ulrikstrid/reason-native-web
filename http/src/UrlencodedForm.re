type key = string;
type value = string;

type t = Uri.t;

let parse = form_data => Uri.of_string("?" ++ form_data);

let get_param = (key: key, t: t) => Uri.get_query_param(t, key);
