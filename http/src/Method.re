let to_string = method => {
  switch (method) {
  | `GET => "GET"
  | `POST => "POST"
  | _ => "other"
  };
};
