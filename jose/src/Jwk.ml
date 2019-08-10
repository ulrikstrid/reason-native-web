type t = {
  alg: string option;
  kty: string;
  use: string option;
  n: string;
  e: string;
  kid: string;
  x5t: string option;
}

let empty = {
  alg = None;
  kty = "";
  use = None;
  n = "";
  e = "";
  kid = "";
  x5t = None;
}

let trim_leading_null s =
  Astring.String.trim ~drop:(function '\000' -> true | _ -> false) s

let make (rsa_pub: Nocrypto.Rsa.pub): (t, [ `Msg of string]) result  =
  let n = (CCString.rev (Z.to_bits rsa_pub.n))
    |> trim_leading_null
    |> Base64.encode ~pad:false ~alphabet:Base64.uri_safe_alphabet in
  let e = (CCString.rev (Z.to_bits rsa_pub.e))
    |> trim_leading_null
    |> Base64.encode ~pad:false ~alphabet:Base64.uri_safe_alphabet in
  let public_key: X509.public_key = `RSA rsa_pub in
  let kid = public_key
    |> X509.key_id
    |> Cstruct.to_string
    |> Base64.encode ~pad:false ~alphabet:Base64.uri_safe_alphabet in
  let x5t = public_key
    |> X509.key_fingerprint
    |> Hex.of_cstruct
    |> Hex.to_bytes
    |> Bytes.to_string
    |> Base64.encode ~pad:false ~alphabet:Base64.uri_safe_alphabet ~len:20 in
    match (n, e, kid, x5t) with
    | (Ok n, Ok e, Ok kid, Ok x5t) ->
      Ok {
        alg = Some "RS256";
        kty = "RSA";
        use = Some "sig";
        n =  n;
        e = e;
        kid = kid;
        x5t = Some x5t;
      }
    | (Ok n, Ok e, Ok kid, _) ->
      Ok {
        alg = Some "RS256";
        kty = "RSA";
        use = Some "sig";
        n =  n;
        e = e;
        kid = kid;
        x5t = None;
      }
    | (Error (`Msg m), _, _, _) -> Error (`Msg ("n " ^ m))
    | (_, Error (`Msg m), _, _) -> Error (`Msg ("e " ^ m))
    | (_, _, Error (`Msg m), _) -> Error (`Msg ("kid " ^ m))

let to_json_from_opt = CCOpt.map_or ~default:`Null Yojson.Basic.from_string

let to_json t = `Assoc [
  ("alg", to_json_from_opt t.alg);
  ("kty", `String t.kty);
  ("use", to_json_from_opt t.use);
  ("n", `String t.n);
  ("e", `String t.e);
  ("kid", `String t.kid);
  ("x5t", to_json_from_opt t.x5t)
]

let from_json json = Yojson.Basic.Util.{
  alg = json |> member "alg" |> to_string_option;
  kty = json |> member "kty" |> to_string;
  use = json |> member "use" |> to_string_option;
  n = json |> member "n" |> to_string;
  e = json |> member "e" |> to_string;
  kid = json |> member "kid" |> to_string;
  x5t = json |> member "x5t" |> to_string_option;
}

let from_string str = Yojson.Basic.from_string(str) |> from_json 