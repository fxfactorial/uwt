(* Lightweight thread library for OCaml
 * http://www.ocsigen.org/lwt
 * Interface Lwt_unix
 * Copyright (C) 2010 Jérémie Dimino
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, with linking exceptions;
 * either version 2.1 of the License, or (at your option) any later
 * version. See COPYING file for details.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *)


(* I/O parts removed, otherwise unmodified import from lwt *)

(** Byte arrays *)

type t = (char, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t
    (** Type of array of bytes. *)

val create : int -> t
  (** Creates a new byte array of the given size. *)

val length : t -> int
  (** Returns the length of the given byte array. *)

(** {2 Access} *)

val get : t -> int -> char
  (** [get buffer offset] returns the byte at offset [offset] in
      [buffer]. *)

val set : t -> int -> char -> unit
  (** [get buffer offset value] changes the value of the byte at
      offset [offset] in [buffer] to [value]. *)

val unsafe_get : t -> int -> char
  (** Same as {!get} but without bound checking. *)

val unsafe_set : t -> int -> char -> unit
  (** Same as {!set} but without bound checking. *)

(** {2 Conversions} *)

val of_bytes : Bytes.t -> t
  (** [of_bytes buf] returns a newly allocated byte array with the
      same contents as [buf]. *)

val of_string : string -> t
  (** [of_string buf] returns a newly allocated byte array with the
      same contents as [buf]. *)

val to_bytes : t -> Bytes.t
  (** [to_bytes buf] returns newly allocated bytes with the same
      contents as [buf]. *)

val to_string : t -> string
  (** [to_string buf] returns a newly allocated string with the same
      contents as [buf]. *)

(** {2 Copying} *)

val blit : t -> int -> t -> int -> int -> unit
  (** [blit buf1 ofs1 buf2 ofs2 len] copy [len] bytes from [buf1]
      starting at offset [ofs1] to [buf2] starting at offset [ofs2]. *)

val blit_from_bytes : Bytes.t -> int -> t -> int -> int -> unit
  (** Same as blit but the first buffer is a string instead of a byte
      array. *)

val blit_to_bytes : t -> int -> Bytes.t -> int -> int -> unit
  (** Same as blit but the second buffer is a string instead of a byte
      array. *)

val unsafe_blit : t -> int -> t -> int -> int -> unit
  (** Same as {!blit} but without bound checking. *)

val unsafe_blit_from_bytes : Bytes.t -> int -> t -> int -> int -> unit
  (** Same as {!blit_string_bytes} but without bound checking. *)

val unsafe_blit_to_bytes : t -> int -> Bytes.t -> int -> int -> unit
  (** Same as {!blit_bytes_string} but without bound checking. *)

val proxy : t -> int -> int -> t
  (** [proxy buffer offset length] creates a ``proxy''. The returned
      byte array share the data of [buffer] but with different
      bounds. *)

val extract : t -> int -> int -> t
  (** [extract buffer offset length] creates a new byte array of
      length [length] and copy the [length] bytes of [buffer] at
      [offset] into it.  *)

val copy : t -> t
  (** [copy buffer] creates a copy of the given byte array. *)

(** {2 Filling} *)

val fill : t -> int -> int -> char -> unit
  (** [fill buffer offset length value] puts [value] in all [length]
      bytes of [buffer] starting at offset [offset]. *)

val unsafe_fill : t -> int -> int -> char -> unit
  (** Same as {!fill} but without bound checking. *)
