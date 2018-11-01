open BsAbstract.Interface;

/********************************************************************************
This is intended to hold modules that are based on list-related module functors.

These are here to avoid circular dependencies between the base modules like List and Option, etc.

E.g.
List.Option.traverse
List.Result.traverse
List.Validation.traverse
********************************************************************************/

module Option {
  module Traversable = BsAbstract.Functors.ListF.Option.Traversable

  let traverse: ('a => option('a), list('a)) => option(list('a)) = Traversable.traverse;

  let sequence: list(option('a)) => option(list('a)) = Traversable.sequence;
}

module Result {
  module Traversable = (Error: TYPE) => BsAbstract.List.Traversable(BsAbstract.Result.Applicative(Error));

  /*
  Traversing with Result has fail fast semantics, and errors are not collected.
  */
  let traverse = (
    type a,
    type e,
    f: a => Belt.Result.t(a, e), /* Each a produces a Result with a success value or a single error value */
    list: list(a)
  ): Belt.Result.t(list(a), e) => {
    module Error = { type t = e };
    module Traversable = Traversable(Error);
    Traversable.traverse(f, list);
  };
}

module Validation {
  module Traversable = (Errors: SEMIGROUP_ANY, Error: TYPE) => BsAbstract.List.Traversable(Validation.Applicative(Errors, Error));
  module TraversableWithErrorsAsList = (Error: TYPE) => Traversable(List.SemigroupAny, Error);
  module TraversableWithErrorsAsListOfStrings = TraversableWithErrorsAsList({ type t = string });
  module TraversableWithErrorsAsNonEmptyList = (Error: TYPE) => Traversable(NonEmpty.List.SemigroupAny, Error);

  /*
  This is a streamlined definition of traverse which allows you to return a Belt.Result.t for each item
  in the list, and all errors are collected in a NonEmpty.List of your error type, using applicative semantics
  for Validation.
  */
  let traverse = (
    type a,
    type e,
    f: a => Belt.Result.t(a, e), /* Each a produces a Result with a success value or a single error value */
    list: list(a)
  ): Validation.t(list(a), NonEmpty.List.t(e)) => {
    module Error = { type t = e };
    module Traversable = Traversable(NonEmpty.List.SemigroupAny, Error);
    /* Map the reuslts to Validation.t(a, NonEmpty.List.t(e)), so we can accumulate the errors */
    Traversable.traverse(a => f(a)->Relude.Result.toValidationNel, list);
  };
}