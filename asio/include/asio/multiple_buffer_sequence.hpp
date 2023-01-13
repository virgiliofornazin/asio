//
// multiple_buffer_sequence.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ( TODO-MBS: update header with copyright of asio C++ library )
//
// Support for multiple datagram buffers code patches on Linux operating system
// Copyright (c) 2023 virgilio A. Fornazin (virgiliofornazin at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_MULTIPLE_BUFFER_SEQUENCE_HPP
#define ASIO_MULTIPLE_BUFFER_SEQUENCE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/array.hpp"
#include "asio/detail/multiple_buffer_sequence_op.hpp"
#include "asio/socket_base.hpp"
#include <stdexcept>
#include <memory>
#include <iterator>
#include <array>
#include <vector>

#include "asio/detail/push_options.hpp"

namespace asio {

static const std::size_t multiple_buffer_sequence_maximum_operations_per_io = 1
#if defined(ASIO_MULTIPLE_BUFFER_SEQUENCE_MAXIMUM_OPERATIONS_PER_IO)
    * ASIO_MULTIPLE_BUFFER_SEQUENCE_MAXIMUM_OPERATIONS_PER_IO
#endif // defined(ASIO_MULTIPLE_BUFFER_SEQUENCE_MAXIMUM_OPERATIONS_PER_IO)
;

template <typename BufferSequence, typename EndpointType,
    typename ContainerType>
class base_multiple_buffer_sequence
{
public:
  typedef base_multiple_buffer_sequence<BufferSequence, EndpointType,
      ContainerType> this_type;

  typedef BufferSequence buffer_sequence_type;
  typedef EndpointType endpoint_type;
  typedef ContainerType container_type;

  typedef detail::multiple_buffer_sequence_op<buffer_sequence_type, 
      endpoint_type> value_type;

protected:
#if defined(ASIO_HAS_STATIC_ASSERT)
  typedef typename container_type::value_type container_value_type; 
  
  typedef typename container_value_type::buffer_sequence_type
      container_buffer_sequence_type;
  
  typedef typename container_value_type::endpoint_type
      container_endpoint_type;

  static_assert(std::is_same<container_buffer_sequence_type,
      buffer_sequence_type>::value, "Container buffer_sequence_type is not the "
      "same as MultipleBufferSequence buffer_sequence_type");

  static_assert(std::is_same<container_endpoint_type,
      endpoint_type>::value, "Container endpoint_type is not the same as "
      "MultipleBufferSequence buffer_sequence_type");

  static_assert(std::is_same<container_value_type,
      value_type>::value, "Container value_type is not the same as "
      "MultipleBufferSequence value_type");
#endif // defined(ASIO_HAS_STATIC_ASSERT)

public:
  typedef typename container_type::size_type size_type;
  typedef typename container_type::reference reference;
  typedef typename container_type::const_reference const_reference;
  typedef typename container_type::pointer pointer;
  typedef typename container_type::const_pointer const_pointer;
  typedef typename container_type::iterator iterator;
  typedef typename container_type::const_iterator const_iterator;
  typedef typename container_type::reverse_iterator reverse_iterator;
  typedef typename container_type::const_reverse_iterator
      const_reverse_iterator;

protected:
  container_type container_;
  std::size_t operations_executed_;
  std::size_t bytes_transferred_;

protected:
  void throw_out_of_range() const
  {
    throw std::out_of_range(
        "multiple buffer sequence size will be greather than maximum "
        "supported operations per io");
  }

  void throw_empty() const
  {
    throw std::out_of_range(
        "no operations were assigned in multiple buffer sequence");
  }
  
public:
  base_multiple_buffer_sequence()
    : operations_executed_(0), bytes_transferred_(0)
  {
  }

  base_multiple_buffer_sequence(const buffer_sequence_type& buffer_sequence)
    : operations_executed_(0), bytes_transferred_(0)
  {
    push_back(buffer_sequence);
  }

  explicit base_multiple_buffer_sequence(
      const buffer_sequence_type& buffer_sequence,
      const endpoint_type& endpoint)
    : operations_executed_(0), bytes_transferred_(0)      
  {
    push_back(buffer_sequence, endpoint);
  }

#if defined(ASIO_HAS_MULTIPLE_BUFFER_SEQUENCE_CONTAINER_COPY)
public:
#else //  defined(ASIO_HAS_MULTIPLE_BUFFER_SEQUENCE_CONTAINER_COPY)
private:
#endif //  defined(ASIO_HAS_MULTIPLE_BUFFER_SEQUENCE_CONTAINER_COPY)

  base_multiple_buffer_sequence(base_multiple_buffer_sequence const& other)
    : container_(other.begin(), other.end()), 
      operations_executed_(other.operations_executed_),
      bytes_transferred_(other.bytes_transferred_)
  {
  }

  base_multiple_buffer_sequence& operator = (
      base_multiple_buffer_sequence const& other)
  {
    container_ = other.container_;
    operations_executed_ = other.operations_executed_;
    bytes_transferred_ = other.bytes_transferred_;
    return (*this);
  }

#if defined(ASIO_HAS_MULTIPLE_BUFFER_SEQUENCE_CONTAINER_MOVE)
public:
#else //  defined(ASIO_HAS_MULTIPLE_BUFFER_SEQUENCE_CONTAINER_MOVE)
private:
#endif //  defined(ASIO_HAS_MULTIPLE_BUFFER_SEQUENCE_CONTAINER_MOVE)
  
#if defined(ASIO_HAS_MOVE)

  base_multiple_buffer_sequence(base_multiple_buffer_sequence&& other)
    : container_(std::move(other)), 
      operations_executed_(std::move(other.operations_executed_)),
      bytes_transferred_(std::move(other.bytes_transferred_))
  {
  }
  
  base_multiple_buffer_sequence& operator = (
      base_multiple_buffer_sequence&& other)
  {
    container_ = std::move(other.container_);
    operations_executed_ = std::move(other.operations_executed_);
    bytes_transferred_ = std::move(other.bytes_transferred_);
    return (*this);
  }

#endif // defined(ASIO_HAS_MOVE)

 public:
  virtual ~base_multiple_buffer_sequence() ASIO_NOEXCEPT
  {
  }
  
  size_type count() const ASIO_NOEXCEPT
  {
    return container_.size();
  }
  
  size_type total_size() const ASIO_NOEXCEPT
  {
    if (empty())
    {
      return 0;
    }

    size_type result = 0;
    size_type const op_count = count();

    for (size_t i = 0; i < op_count; ++i)
    {
      const_reference buffer_sequence = at(i);

      result += buffer_sequence.total_size();
    }
    
    return result;
  }

  bool all_empty() const
  {
    if (empty())
    {
      return true;
    }

    size_type const op_count = count();

    for (size_t i = 0; i < op_count; ++i)
    {
      const_reference buffer_sequence = at(i);

      if (!buffer_sequence.all_empty())
      {
        return false;
      }
    }
    
    return true;
  }

  void reset()
  {
    size_type const op_count = count();

    for (size_t i = 0; i < op_count; ++i)
    {
      reference buffer_sequence = container_.at(i);

      buffer_sequence.reset();
    }
  }

  bool full() const ASIO_NOEXCEPT
  {
    return (size() >= max_size());
  }

  void throw_if_empty() const
  {
    if (empty())
    {
      throw_empty();
    }
  }

  reference at(size_type index)
  {
    return container_.at(index);
  }

  const_reference at(size_type index) const
  {
    return container_.at(index);
  }

  reference operator[](std::size_t index)
  {
    return container_[index];
  }

  const_reference operator[](std::size_t index) const
  {
    return container_[index];
  }

  reference front()
  {
    return container_.front();
  }

  const_reference front() const
  {
    return container_.front();
  }

  reference back()
  {
    return container_.back();
  }

  const_reference back() const
  {
    return container_.back();
  }

  pointer data() ASIO_NOEXCEPT
  {
    return container_.data();
  }

  const_pointer data() const ASIO_NOEXCEPT
  {
    return container_.data();
  }

  iterator begin() ASIO_NOEXCEPT
  {
    return container_.begin();
  }

  const_iterator begin() const ASIO_NOEXCEPT
  {
    return container_.begin();
  }

  const_iterator cbegin() const ASIO_NOEXCEPT
  {
    return container_.cbegin();
  }

  iterator end() ASIO_NOEXCEPT
  {
    return container_.end();
  }

  const_iterator end() const ASIO_NOEXCEPT
  {
    return container_.end();
  }

  const_iterator cend() const ASIO_NOEXCEPT
  {
    return container_.cend();
  }

  reverse_iterator rbegin() ASIO_NOEXCEPT
  {
    return container_.rbegin();
  }

  const_reverse_iterator rbegin() const ASIO_NOEXCEPT
  {
    return container_.rbegin();
  }

  const_reverse_iterator crbegin() const ASIO_NOEXCEPT
  {
    return container_.crbegin();
  }

  reverse_iterator rend() ASIO_NOEXCEPT
  {
    return container_.rend();
  }

  const_reverse_iterator rend() const ASIO_NOEXCEPT
  {
    return container_.rend();
  }

  const_reverse_iterator crend() const ASIO_NOEXCEPT
  {
    return container_.crend();
  }

  bool empty() const ASIO_NOEXCEPT
  {
    return container_.empty();
  }

  size_type size() const ASIO_NOEXCEPT
  {
    return container_.size();
  }

  size_type max_size() const ASIO_NOEXCEPT
  {
    return multiple_buffer_sequence_maximum_operations_per_io;
  }

  void swap(base_multiple_buffer_sequence<BufferSequence, EndpointType, 
      ContainerType>& other)
  {
    std::swap(container_, other.container_);
    std::swap(operations_executed_, other.operations_executed_);
    std::swap(bytes_transferred_, other.bytes_transferred_);
  }

  std::size_t operations_executed() const ASIO_NOEXCEPT
  {
    return operations_executed_;
  }

  void set_operations_executed(std::size_t _operations_executed) ASIO_NOEXCEPT
  {
    operations_executed_ = _operations_executed;
  }

  std::size_t add_operations_executed(std::size_t _operations_executed = 1) ASIO_NOEXCEPT
  {
    std::size_t prev = operations_executed_;

    operations_executed_ += _operations_executed;

    return prev;
  }

  std::size_t bytes_transferred() const ASIO_NOEXCEPT
  {
    return bytes_transferred_;
  }

  void set_bytes_transferred(std::size_t _bytes_transferred) ASIO_NOEXCEPT
  {
    bytes_transferred_ = _bytes_transferred;
  }

  std::size_t add_bytes_transferred(std::size_t _bytes_transferred) ASIO_NOEXCEPT
  {
    std::size_t prev = bytes_transferred_;

    bytes_transferred_ += _bytes_transferred;

    return prev;
  }
};

template <typename BufferSequence, typename EndpointType, 
    std::size_t BufferSequenceCount>
class fixed_size_multiple_buffer_sequence
  : public base_multiple_buffer_sequence<BufferSequence, EndpointType,
  detail::array<detail::multiple_buffer_sequence_op<BufferSequence, EndpointType>, 
    BufferSequenceCount>>
{
public:
  typedef fixed_size_multiple_buffer_sequence<BufferSequence, EndpointType,
      BufferSequenceCount> this_type;

  typedef base_multiple_buffer_sequence<BufferSequence, EndpointType,
      detail::array<detail::multiple_buffer_sequence_op<BufferSequence,
      EndpointType>, BufferSequenceCount>> base_type;

  typedef typename base_type::buffer_sequence_type buffer_sequence_type;
  typedef typename base_type::endpoint_type endpoint_type;
  typedef typename base_type::value_type value_type;

  typedef typename base_type::size_type size_type;
  typedef typename base_type::reference reference;
  typedef typename base_type::const_reference const_reference;
  typedef typename base_type::pointer pointer;
  typedef typename base_type::const_pointer const_pointer;
  typedef typename base_type::iterator iterator;
  typedef typename base_type::const_iterator const_iterator;
  typedef typename base_type::reverse_iterator reverse_iterator;
  typedef typename base_type::const_reverse_iterator const_reverse_iterator;
};

template <typename BufferSequence, typename EndpointType>
static inline fixed_size_multiple_buffer_sequence<BufferSequence, EndpointType,
    1> make_fixed_size_multiple_buffer_sequence(
        const BufferSequence& buffer_sequence, const EndpointType& endpoint)
{
  return fixed_size_multiple_buffer_sequence<BufferSequence, EndpointType, 1>(
    buffer_sequence, endpoint);
};

template <typename BufferSequence, typename EndpointType,
    typename BufferSequenceContainerAllocatorType = std::allocator<
    detail::multiple_buffer_sequence_op<BufferSequence, EndpointType>>>
class resizeable_multiple_buffer_sequence
  : public base_multiple_buffer_sequence<BufferSequence, EndpointType,
  std::vector<detail::multiple_buffer_sequence_op<BufferSequence, 
    EndpointType>, BufferSequenceContainerAllocatorType>>
{
public:
  typedef resizeable_multiple_buffer_sequence<BufferSequence, EndpointType,
      BufferSequenceContainerAllocatorType> this_type;

  typedef base_multiple_buffer_sequence<BufferSequence, EndpointType,
      std::vector<detail::multiple_buffer_sequence_op<BufferSequence, 
      EndpointType>, BufferSequenceContainerAllocatorType>> base_type;

  typedef typename base_type::buffer_sequence_type buffer_sequence_type;
  typedef typename base_type::endpoint_type endpoint_type;
  typedef typename base_type::value_type value_type;

  typedef typename base_type::size_type size_type;
  typedef typename base_type::reference reference;
  typedef typename base_type::const_reference const_reference;
  typedef typename base_type::pointer pointer;
  typedef typename base_type::const_pointer const_pointer;
  typedef typename base_type::iterator iterator;
  typedef typename base_type::const_iterator const_iterator;
  typedef typename base_type::reverse_iterator reverse_iterator;
  typedef typename base_type::const_reverse_iterator const_reverse_iterator;

public:
  void throw_if_overflow(size_type new_size) const
  {
    if (new_size > multiple_buffer_sequence_maximum_operations_per_io)
    {
      this->throw_out_of_range();
    }
  }

  void throw_if_full() const
  {
    if (this->size() == multiple_buffer_sequence_maximum_operations_per_io)
    {
      this->throw_out_of_range();
    }
  }

  void reserve(size_type count)
  {
    throw_if_overflow(count);

    this->container_.reserve(count);
  }

  size_type capacity() const ASIO_NOEXCEPT
  {
    return this->container_.capacity() > 
        multiple_buffer_sequence_maximum_operations_per_io ? 
        multiple_buffer_sequence_maximum_operations_per_io : 
        this->container_.capacity();
  }

  void shrink_to_fit() const
  {
    return this->container_.shrink_to_fit();
  }

  void clear()
  {
    this->container_.clear();
  }

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)
  template <typename... Args>
  void insert(const_iterator pos, Args&& ... args)
  {
    throw_if_full();
    
    this->container_.emplace(pos, value_type(std::forward<Args>(args)...));
  }

  template <typename... Args>
  void push_back(Args&& ... args)
  {
    throw_if_full();
    
    this->container_.emplace_back(value_type(std::forward<Args>(args)...));
  }
#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)
  void insert(const_iterator pos, const buffer_sequence_type& buffer_sequence)
  {
    throw_if_full();
    
    this->container_.insert(pos, value_type(buffer_sequence);
  }

  void insert(const_iterator pos, const buffer_sequence_type& buffer_sequence,
      const endpoint_type& endpoint)
  {
    throw_if_full();
    
    this->container_.insert(pos, value_type(buffer_sequence, endpoint));
  }

  void insert(const_iterator pos, const buffer_sequence_type& buffer_sequence,
      const endpoint_type& endpoint, socket_base::message_flags flags)
  {
    throw_if_full();
    
    this->container_.insert(pos, value_type(buffer_sequence, endpoint, flags));
  }

  void push_back(const_iterator pos,
      const buffer_sequence_type& buffer_sequence)
  {
    throw_if_full();
    
    this->container_.push_back(pos, value_type(buffer_sequence);
  }

  void push_back(const_iterator pos,
      const buffer_sequence_type& buffer_sequence,
      const endpoint_type& endpoint)
  {
    throw_if_full();
    
    this->container_.push_back(pos, value_type(buffer_sequence, endpoint));
  }

  void push_back(const_iterator pos, 
      const buffer_sequence_type& buffer_sequence,
      const endpoint_type& endpoint, socket_base::message_flags flags)
  {
    throw_if_full();

    this->container_.push_back(pos, value_type(buffer_sequence, endpoint, flags));
  }
#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)

  void erase(iterator pos)
  {
    this->container_.erase(pos);
  }

  void erase(const_iterator pos)
  {
    this->container_.erase(pos);
  }

  void erase(iterator first, iterator last)
  {
    this->container_.erase(first, last);
  }

  void erase(const_iterator first, const_iterator last)
  {
    this->container_.erase(first, last);
  }

  void pop_back()
  {
    this->container_.pop_back();
  }

  void resize(size_type count)
  {
    throw_if_overflow(count);

    size_type const previous_size = this->size();

    this->container_.resize(count);
  }
};

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_MULTIPLE_BUFFER_SEQUENCE_HPP
