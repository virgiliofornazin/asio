//
// detail/reactive_socket_sendmmsg_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ( TODO: update header with copyright of asio C++ library )
//
// Support for multiple datagram buffers code patches on Linux operating system
// Copyright (c) 2023 virgilio A. Fornazin (virgiliofornazin at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_REACTIVE_SOCKET_SENDMMSG_OP_HPP
#define ASIO_DETAIL_REACTIVE_SOCKET_SENDMMSG_OP_HPP

#include "asio/buffer.hpp"
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_MULTIPLE_DATAGRAM_BUFFER_IO)

#include "asio/detail/bind_handler.hpp"
#include "asio/multiple_datagram_buffers.hpp"
#include "asio/detail/fenced_block.hpp"
#include "asio/detail/handler_alloc_helpers.hpp"
#include "asio/detail/handler_invoke_helpers.hpp"
#include "asio/detail/handler_work.hpp"
#include "asio/detail/memory.hpp"
#include "asio/detail/reactor_op.hpp"
#include "asio/detail/socket_ops.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename ConstBufferSequence, typename EndpointType>
class reactive_socket_sendmmsg_op_base : public reactor_op
{
public:
  reactive_socket_sendmmsg_op_base(const asio::error_code& success_ec,
      socket_type socket, socket_ops::state_type state,
      multiple_datagram_buffers<ConstBufferSequence, EndpointType>& buffers,
      socket_base::message_flags flags, func_type complete_func)
    : reactor_op(success_ec,
        &reactive_socket_sendmmsg_op_base::do_perform, complete_func),
      socket_(socket),
      state_(state),
      buffers_(buffers),
      flags_(flags)
  {
  }

  static status do_perform(reactor_op* base)
  {
    reactive_socket_sendmmsg_op_base* o(
        static_cast<reactive_socket_sendmmsg_op_base*>(base));
/*
    typedef buffer_sequence_adapter<asio::const_buffer,
        ConstBufferSequence> bufs_type;

    status result;
    if (bufs_type::is_single_buffer)
    {
      result = socket_ops::non_blocking_send1(o->socket_,
          bufs_type::first(o->buffers_).data(),
          bufs_type::first(o->buffers_).size(), o->flags_,
          o->ec_, o->bytes_transferred_) ? done : not_done;

      if (result == done)
        if ((o->state_ & socket_ops::stream_oriented) != 0)
          if (o->bytes_transferred_ < bufs_type::first(o->buffers_).size())
            result = done_and_exhausted;
    }
    else
    {
      bufs_type bufs(o->buffers_);
      result = socket_ops::non_blocking_send(o->socket_,
            bufs.buffers(), bufs.count(), o->flags_,
            o->ec_, o->bytes_transferred_) ? done : not_done;

      if (result == done)
        if ((o->state_ & socket_ops::stream_oriented) != 0)
          if (o->bytes_transferred_ < bufs.total_size())
            result = done_and_exhausted;
    }
*/
    status result{};

    ASIO_HANDLER_REACTOR_OPERATION((*o, "non_blocking_sendmmsg",
          o->ec_, o->bytes_transferred_));

    return result;
  }

private:
  socket_type socket_;
  socket_ops::state_type state_;
  multiple_datagram_buffers<ConstBufferSequence, EndpointType> buffers_;
  socket_base::message_flags flags_;
};

template <typename ConstBufferSequence, typename EndpointType, typename Handler, typename IoExecutor>
class reactive_socket_sendmmsg_op :
  public reactive_socket_sendmmsg_op_base<ConstBufferSequence, EndpointType>
{
public:
  ASIO_DEFINE_HANDLER_PTR(reactive_socket_sendmmsg_op);

  reactive_socket_sendmmsg_op(const asio::error_code& success_ec,
      socket_type socket, socket_ops::state_type state,
      multiple_datagram_buffers<ConstBufferSequence, EndpointType>& buffers,
      socket_base::message_flags flags,
      Handler& handler, const IoExecutor& io_ex)
    : reactive_socket_sendmmsg_op_base<ConstBufferSequence, EndpointType>(success_ec, socket,
        state, buffers, flags, &reactive_socket_sendmmsg_op::do_complete),
      handler_(ASIO_MOVE_CAST(Handler)(handler)),
      work_(handler_, io_ex)
  {
  }

  static void do_complete(void* owner, operation* base,
      const asio::error_code& /*ec*/,
      std::size_t /*bytes_transferred*/)
  {
    // Take ownership of the handler object.
    reactive_socket_sendmmsg_op* o(static_cast<reactive_socket_sendmmsg_op*>(base));
    ptr p = { asio::detail::addressof(o->handler_), o, o };

    ASIO_HANDLER_COMPLETION((*o));

    // Take ownership of the operation's outstanding work.
    handler_work<Handler, IoExecutor> w(
        ASIO_MOVE_CAST2(handler_work<Handler, IoExecutor>)(
          o->work_));

    ASIO_ERROR_LOCATION(o->ec_);

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    detail::binder2<Handler, asio::error_code, std::size_t>
      handler(o->handler_, o->ec_, o->bytes_transferred_);
    p.h = asio::detail::addressof(handler.handler_);
    p.reset();

    // Make the upcall if required.
    if (owner)
    {
      fenced_block b(fenced_block::half);
      ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_, handler.arg2_));
      w.complete(handler, handler.handler_);
      ASIO_HANDLER_INVOCATION_END;
    }
  }

private:
  Handler handler_;
  handler_work<Handler, IoExecutor> work_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_MULTIPLE_DATAGRAM_BUFFER_IO)

#endif // ASIO_DETAIL_REACTIVE_SOCKET_SENDMMSG_OP_HPP
