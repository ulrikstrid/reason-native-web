type key = string;
type value = string;
type t;

let parse: string => t;
let get_param: (key, t) => option(value);
