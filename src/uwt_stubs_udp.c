/* Libuv bindings for OCaml
 * http://github.com/fdopen/uwt
 * Copyright (C) 2015-2016 Andreas Hauptmann
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
 */

#include "uwt_stubs_udp.h"

/* some functions are defined inside uwt_stubs_tcp */

static const int udp_bin_flag_table[2] = {
  UV_UDP_IPV6ONLY, UV_UDP_REUSEADDR
};

CAMLprim value
uwt_udp_bind_na(value o_udp, value o_sock, value o_flags){
  struct sockaddr_storage addr;
  if ( !uwt__get_sockaddr(o_sock, (struct sockaddr *) &addr)){
    return VAL_UWT_INT_RESULT_UNKNOWN;
  }
  HANDLE_NINIT_NA(t,o_udp);
  const unsigned int flags = SAFE_CONVERT_FLAG_LIST(o_flags,udp_bin_flag_table);
  const int ret = uv_udp_bind((uv_udp_t *)t->handle,
                              (struct sockaddr *)&addr,
                              flags);
  if ( ret >= 0 ){
    t->initialized = 1;
  }
  return (VAL_UWT_UNIT_RESULT(ret));
}

CAMLprim value
uwt_udp_set_membership_na(value o_udp, value o_mul,
                          value o_int, value o_mem)
{
  HANDLE_NINIT_NA(u,o_udp);
  HANDLE_NO_UNINIT_NA(u);
  uv_membership membership = Long_val(o_mem) ? UV_JOIN_GROUP : UV_LEAVE_GROUP;
  const char* multicast_addr = String_val(o_mul);
  char* interface_addr = NULL;
  if ( !uwt_is_safe_string(o_mul) ){
    return VAL_UWT_INT_RESULT_ECHARSET;
  }
  if ( o_int != Val_unit ){
    value s = Field(o_int,0);
    if ( !uwt_is_safe_string(s) ){
      return VAL_UWT_INT_RESULT_ECHARSET;
    }
    interface_addr = String_val(s);
  }
  int ret = uv_udp_set_membership((uv_udp_t*)u->handle,
                                  multicast_addr,
                                  interface_addr,
                                  membership);
  return (VAL_UWT_UNIT_RESULT(ret));
}

CAMLprim value
uwt_udp_set_multicast_loop_na(value o_udp, value o_b)
{
  HANDLE_NINIT_NA(u,o_udp);
  HANDLE_NO_UNINIT_NA(u);
  int ret = uv_udp_set_multicast_loop((uv_udp_t*)u->handle,Long_val(o_b));
  return (VAL_UWT_UNIT_RESULT(ret));
}

CAMLprim value
uwt_udp_set_multicast_ttl_na(value o_udp, value o_ttl)
{
  HANDLE_NINIT_NA(u,o_udp);
  HANDLE_NO_UNINIT_NA(u);
  int ttl = Long_val(o_ttl);
  if ( ttl < 1 || ttl > 255 ){
    return VAL_UWT_INT_RESULT_EINVAL;
  }
  int ret = uv_udp_set_multicast_ttl((uv_udp_t*)u->handle,ttl);
  return (VAL_UWT_UNIT_RESULT(ret));
}

CAMLprim value
uwt_udp_set_multicast_interface_na(value o_udp, value o_inter)
{
  HANDLE_NINIT_NA(u,o_udp);
  HANDLE_NO_UNINIT_NA(u);
  char * iface = NULL;
  if ( o_inter != Val_unit ){
    value s = Field(o_inter,0);
    if ( !uwt_is_safe_string(s) ){
      return VAL_UWT_INT_RESULT_ECHARSET;
    }
    iface = String_val(s);
  }
  int ret = uv_udp_set_multicast_interface((uv_udp_t*)u->handle,iface);
  return (VAL_UWT_UNIT_RESULT(ret));
}

CAMLprim value
uwt_udp_set_broadcast_na(value o_udp, value o_b)
{
  HANDLE_NINIT_NA(u,o_udp);
  HANDLE_NO_UNINIT_NA(u);
  int ret = uv_udp_set_broadcast((uv_udp_t*)u->handle,Long_val(o_b));
  return (VAL_UWT_UNIT_RESULT(ret));
}

CAMLprim value
uwt_udp_set_ttl_na(value o_udp, value o_ttl)
{
  HANDLE_NINIT_NA(u,o_udp);
  HANDLE_NO_UNINIT_NA(u);
  int ttl = Long_val(o_ttl);
  int ret;
  if ( ttl < 1 || ttl > 255 ){
    return VAL_UWT_INT_RESULT_EINVAL;
  }
  ret = uv_udp_set_ttl((uv_udp_t*)u->handle,ttl);
  return (VAL_UWT_UNIT_RESULT(ret));
}

/*
| Data of ( Bytes.t * sockaddr option)
| Partial_data of ( Bytes.t * sockaddr option)
| Empty_from of sockaddr
| Transmission_error of error
*/
#define Data_of 0
#define Partial_data 1
#define Empty_from 2
#define Transmission_error 3
static value
alloc_recv_result(ssize_t nread,
                  const uv_buf_t* buf,
                  const struct sockaddr* addr,
                  unsigned int flags)
{
  CAMLparam0();
  CAMLlocal3(param,msock_addr,bytes_t);
  if ( nread > 0 && buf->base ){
    if ( (size_t)nread > buf->len ){
      param = caml_alloc_small(1,Transmission_error);
      Field(param,0) = VAL_UWT_ERROR_UWT_EFATAL;
    }
    else {
      int tag;
      bytes_t = caml_alloc_string(nread);
      memcpy(String_val(bytes_t),buf->base,nread);
      if ( addr == NULL ){
        msock_addr = Val_unit;
      }
      else {
        param = uwt__alloc_sockaddr(addr);
        if ( param == Val_unit ){
          msock_addr = Val_unit;
        }
        else {
          msock_addr = caml_alloc_small(1,Some_tag);
          Field(msock_addr,0) = param;
        }
      }
      if ( (flags & UV_UDP_PARTIAL ) != 0 ){
        tag = Partial_data;
      }
      else {
        tag = Data_of;
      }
      param = caml_alloc_small(2,tag);
      Field(param,0) = bytes_t;
      Field(param,1) = msock_addr;
    }
  }
  else if ( nread == 0 ){
    msock_addr = uwt__alloc_sockaddr(addr);
    if ( msock_addr == Val_unit ){
      param = caml_alloc_small(1,Transmission_error);
      Field(param,0) = VAL_UWT_ERROR_UNKNOWN;
    }
    else {
      param = caml_alloc_small(1,Empty_from);
      Field(param,0) = msock_addr;
    }
  }
  else {
    param = caml_alloc_small(1,Transmission_error);
    Field(param,0) = Val_uwt_error(nread);
  }
  CAMLreturn(param);
}
#undef Data_of
#undef Partial_data
#undef Empty_from
#undef Transmission_error

static void
uwt_udp_recv_cb(uv_udp_t* handle,
                ssize_t nread,
                const uv_buf_t* buf,
                const struct sockaddr* addr,
                unsigned int flags)
{
  HANDLE_CB_INIT_WITH_CLEAN(handle);
  value exn = Val_unit;
  bool buf_not_cleaned = true;
  struct handle * uh = handle->data;
  if ( uh->close_called == 0 ){
    if ( uh->cb_read == CB_INVALID ){
      DEBUG_PF("callback lost");
    }
    else {
      /* nread == 0 && addr == NULL only means we need to clear
         the buffer */
      if ( nread != 0 || addr != NULL ){
        value p = alloc_recv_result(nread,buf,addr,flags);
        if ( nread > 0 ){
          buf_not_cleaned = false;
          uwt__free_uv_buf_t_const(buf,uh->cb_type);
        }
        exn = GET_CB_VAL(uh->cb_read);
        exn = caml_callback_exn(exn,p);
      }
    }
  }
  if ( buf_not_cleaned && buf->base ){
    uwt__free_uv_buf_t_const(buf,uh->cb_type);
  }
  HANDLE_CB_RET(exn);
}

CAMLprim value
uwt_udp_recv_start(value o_udp, value o_cb)
{
  HANDLE_NO_UNINIT_CLOSED_INT_RESULT(o_udp);
  HANDLE_INIT2(u,o_udp,o_cb);
  value ret;
  if ( u->cb_read != CB_INVALID ){
    ret = VAL_UWT_INT_RESULT_EBUSY;
  }
  else {
    int erg = 0;
    uv_udp_t* ux = (uv_udp_t*)u->handle;
    if ( u->can_reuse_cb_read == 1 ){
      u->can_reuse_cb_read = 0;
      u->read_waiting = 0;
      erg = uv_udp_recv_stop(ux);
    }
    if ( erg >= 0 ){
      erg = uv_udp_recv_start(ux,
                              uwt__alloc_cb,
                              uwt_udp_recv_cb);
      if ( erg >= 0 ){
        u->c_read_size = DEF_ALLOC_SIZE;
        uwt__gr_register(&u->cb_read,o_cb);
        ++u->in_use_cnt;
      }
    }
    ret = VAL_UWT_UNIT_RESULT(erg);
  }
  CAMLreturn(ret);
}

CAMLprim value
uwt_udp_recv_stop(value o_udp, value o_abort)
{
  HANDLE_NO_UNINIT_CLOSED_INT_RESULT(o_udp);
  HANDLE_INIT(u,o_udp);
  value ret;
  if ( u->cb_read == CB_INVALID ||
       (u->read_waiting == 1 && Long_val(o_abort) == 0 )){
    /* Yes, dubious, but upstream libuv also doesn't report any error */
    ret = Val_long(0);
  }
  else {
    const int erg = uv_udp_recv_stop((uv_udp_t*)u->handle);
    if ( erg >= 0 ){
      u->can_reuse_cb_read = 0;
      if ( u->in_use_cnt ){
        --u->in_use_cnt;
      }
      uwt__gr_unregister(&u->cb_read);
    }
    ret = VAL_UWT_UNIT_RESULT(erg);
  }
  CAMLreturn(ret);
}

static void
uwt_udp_recv_own_cb(uv_udp_t* handle,
                    ssize_t nread,
                    const uv_buf_t* buf,
                    const struct sockaddr* addr,
                    unsigned int flags)
{
  HANDLE_CB_INIT_WITH_CLEAN(handle);
  value exn = Val_unit;
  bool buf_not_cleaned = true;
  struct handle * uh = handle->data;
  const int read_ba = uh->use_read_ba;
  if ( uh->close_called == 0 ){
    if (unlikely( uh->cb_read == CB_INVALID ||
                  uh->obuf == CB_INVALID )){
      DEBUG_PF("callback lost");
    }
    else {
      /* nread == 0 && addr == NULL only means we need to clear
         the buffer */
      if ( nread != 0 || addr != NULL ){
        value param;
        uh->read_waiting = 0;
        if ( nread < 0 ){
          param = caml_alloc_small(1,Error_tag);
          Field(param,0) = Val_uwt_error(nread);
        }
        else {
          value triple = Val_unit;
          value sockaddr = Val_unit;
          value is_partial;
          param = Val_unit;
          Begin_roots3(triple,sockaddr,param);
          if ( addr != NULL ){
            param = uwt__alloc_sockaddr(addr);
            if ( param != Val_unit ){
              sockaddr = caml_alloc_small(1,Some_tag);
              Field(sockaddr,0) = param;
            }
          }
          if ( flags & UV_UDP_PARTIAL ){
            is_partial = Val_long(1);
          }
          else {
            is_partial = Val_long(0);
          }
          if ( nread != 0 && read_ba == 0 ){
            value o = GET_CB_VAL(uh->obuf);
            assert( Tag_val(o) == String_tag );
            size_t len = UMIN(uh->c_read_size,(size_t)nread);
            memcpy(String_val(o) + uh->obuf_offset,
                   buf->base,
                   len);
            buf_not_cleaned = false;
            uwt__free_uv_buf_t_const(buf,uh->cb_type);
          }
          triple = caml_alloc_small(3,0);
          Field(triple,0) = Val_long(nread);
          Field(triple,1) = is_partial;
          Field(triple,2) = sockaddr;
          param = caml_alloc_small(1,Ok_tag);
          Field(param,0) = triple;
          End_roots();
        }
        exn = GET_CB_VAL(uh->cb_read);
        uh->can_reuse_cb_read = 1;
        uwt__gr_unregister(&uh->cb_read);
        uwt__gr_unregister(&uh->obuf);
        if ( uh->in_use_cnt ){
          uh->in_use_cnt--;
        }
        exn = caml_callback2_exn(*uwt__global_wakeup,exn,param);
        if ( uh->close_called == 0 && uh->can_reuse_cb_read == 1 ){
          uv_udp_recv_stop(handle);
          uh->can_reuse_cb_read = 0;
        }
      }
    }
  }
  if ( read_ba == 0 && buf_not_cleaned && buf->base ){
    uwt__free_uv_buf_t_const(buf,uh->cb_type);
  }
  HANDLE_CB_RET(exn);
}

CAMLprim value
uwt_udp_recv_own(value o_udp,value o_buf,value o_offset,value o_len,value o_cb)
{
  HANDLE_NO_UNINIT_CLOSED_INT_RESULT(o_udp);
  HANDLE_INIT3(u,o_udp,o_buf,o_cb);
  const int ba = Tag_val(o_buf) != String_tag;
  size_t len = Long_val(o_len);
  value ret;
  if ( u->cb_read != CB_INVALID ){
    ret = VAL_UWT_INT_RESULT_EBUSY;
  }
  else if ( len > ULONG_MAX ){
    ret = VAL_UWT_INT_RESULT_EINVAL;
  }
  else {
    int erg;
    uv_udp_t* ux = (uv_udp_t*)u->handle;
    if ( u->can_reuse_cb_read == 1 ){
      u->can_reuse_cb_read = 0;
      erg = 0;
    }
    else {
      erg = uv_udp_recv_start(ux,uwt__alloc_own_cb,uwt_udp_recv_own_cb);
    }
    if ( erg >= 0 ){
      size_t offset = Long_val(o_offset);
      uwt__gr_register(&u->cb_read,o_cb);
      uwt__gr_register(&u->obuf,o_buf);
      ++u->in_use_cnt;
      u->c_read_size = len;
      u->use_read_ba = ba;
      u->read_waiting = 1;
      if ( ba == 0 ){
        u->obuf_offset = offset;
      }
      else {
        u->ba_read = Ba_buf_val(o_buf) + offset;
      }
    }
    ret = VAL_UWT_UNIT_RESULT(erg);
  }
  CAMLreturn(ret);
}

CAMLprim value
uwt_udp_send_queue_size_na(value o_udp)
{
  value ret;
  struct handle * h = Handle_val(o_udp);
  if ( HANDLE_IS_INVALID_UNINIT(h) ){
    ret = Val_long(0);
  }
  else {
    uv_udp_t* u = (uv_udp_t*)h->handle;
    ret = Val_long((intnat)u->send_queue_size);
  }
  return ret;
}

CAMLprim value
uwt_udp_send_queue_count_na(value o_udp)
{
  value ret;
  struct handle * h = Handle_val(o_udp);
  if ( HANDLE_IS_INVALID_UNINIT(h) ){
    ret = Val_long(0);
  }
  else {
    uv_udp_t* u = (uv_udp_t*)h->handle;
    ret = Val_long((intnat)u->send_queue_count);
  }
  return ret;
}
