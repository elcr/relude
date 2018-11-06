open BsAbstract.Interface;

type t('a, 'e) =
  | Idle
  | Loading
  | Loaded('a)
  | Refreshing('a)
  | Failed('e);

let pure: 'a => t('a, 'e) = a => Loaded(a);

let idle: t('a, 'e) = Idle;
let loading: t('a, 'e) = Loading;
let loaded: 'a => t('a, 'e) = a => Loaded(a);
let refreshing: 'a => t('a, 'e) = a => Refreshing(a);
let failed: 'e => t('a, 'e) = e => Refreshing(e);

let map: ('a => 'b, t('a, 'e)) => t('b, 'e) =
  (f, fa) =>
    switch (fa) {
    | Idle => Idle
    | Loading => Loading
    | Loaded(a) => Loaded(f(a))
    | Refreshing(a) => Refreshing(f(a))
    | Failed(e) => Failed(e)
    };

let mapError: ('e1 => 'e2, t('a, 'e1)) => t('a, 'e2) =
  (f, fa) =>
    switch (fa) {
    | Idle => Idle
    | Loading => Loading
    | Loaded(_) as loaded => loaded
    | Refreshing(_) as refreshing => refreshing
    | Failed(e) => Failed(f(e))
    };

/* TODO: maybe we want to require a SEMIGROUP to collect errors for the Failed case */
let apply: (t('a => 'b, 'e), t('a, 'e)) => t('b, 'e) =
  (ff, fa) =>
    switch (ff, fa) {
    | (Idle, Idle) => Idle
    | (Idle, Loading) => Loading
    | (Idle, Loaded(_)) => Idle
    | (Idle, Refreshing(_)) => Idle
    | (Idle, Failed(e2)) => Failed(e2)

    | (Loading, Idle) => Loading
    | (Loading, Loading) => Loading
    | (Loading, Loaded(_)) => Loading
    | (Loading, Refreshing(_)) => Loading
    | (Loading, Failed(e2)) => Failed(e2)

    | (Loaded(_), Idle) => Idle
    | (Loaded(_), Loading) => Loading
    | (Loaded(f), Loaded(a)) => Loaded(f(a))
    | (Loaded(f), Refreshing(a)) => Refreshing(f(a))
    | (Loaded(_), Failed(e2)) => Failed(e2)

    | (Refreshing(_), Idle) => Idle
    | (Refreshing(_), Loading) => Loading
    | (Refreshing(f), Refreshing(a)) => Refreshing(f(a))
    | (Refreshing(f), Loaded(a)) => Refreshing(f(a))
    | (Refreshing(_), Failed(e2)) => Failed(e2)

    | (Failed(e1), Idle) => Failed(e1)
    | (Failed(e1), Loading) => Failed(e1)
    | (Failed(e1), Loaded(_)) => Failed(e1)
    | (Failed(e1), Refreshing(_)) => Failed(e1)
    | (Failed(_), Failed(e2)) => Failed(e2)
    };

let flatMap: (t('a, 'e), 'a => t('b, 'e)) => t('b, 'e) =
  (fa, f) =>
    switch (fa) {
    | Idle => Idle
    | Loading => Loading
    | Loaded(a) => f(a)
    | Refreshing(a) => f(a)
    | Failed(e) => Failed(e)
    };

module type FUNCTOR = (E: TYPE) => FUNCTOR with type t('a) = t('a, E.t);

module Functor: FUNCTOR =
  (E: TYPE) => {
    type nonrec t('a) = t('a, E.t);
    let map = map;
  };

module type APPLY = (E: TYPE) => APPLY with type t('a) = t('a, E.t);

module Apply: APPLY =
  (E: TYPE) => {
    include Functor(E);
    let apply = apply;
  };

/* Provides the map2/map3/etc. functions, but requires an Error type */
module AsyncDataApply = (Error: TYPE) =>
  BsAbstract.Functions.Apply((Apply(Error)));

module type APPLICATIVE =
  (E: TYPE) => APPLICATIVE with type t('a) = t('a, E.t);

module Applicative: APPLICATIVE =
  (E: TYPE) => {
    include Apply(E);
    let pure = pure;
  };

module type MONAD = (E: TYPE) => MONAD with type t('a) = t('a, E.t);

module Monad: MONAD =
  (E: TYPE) => {
    include Applicative(E);
    let flat_map = flatMap;
  };
