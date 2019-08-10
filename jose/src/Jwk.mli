type t = {
  alg: string option;
  kty: string;
  use: string option;
  n: string;
  e: string;
  kid: string;
  x5t: string option;
}

val empty : t

val make : Nocrypto.Rsa.pub -> (t, [ `Msg of string]) result   

val to_json : t -> Yojson.Basic.t

val from_json : Yojson.Basic.t -> t

val from_string : string -> t