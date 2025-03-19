#include "reassembler.hh"
#include "debug.hh"
#include<ranges>
using namespace std;


auto Reassembler::split(uint64_t pos) noexcept
{
  auto it  {buf_.lower_bound(pos)};  
  if(it != buf_.end() and it->first == pos){
    return it;
  }
  if(it == buf_.begin()){
    return it;
  }
  if(const auto pit {prev(it)}; pit ->first + size(pit->second) > pos){
    const auto res {buf_.emplace_hint(it,pos,pit->second.substr(pos - pit->first))};
    pit->second.resize(pos - pit->first);
    return res;
  }
  return it; //return the first bigger or equal to it than pos
}


void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  const auto try_close = [&]() noexcept -> void{
    if(end_index_.has_value() and end_index_.value() == writer().bytes_pushed()){
      output_.writer().close();
    }
  };

  if(data.empty()){
    if(not end_index_.has_value() and is_last_substring){
      end_index_.emplace(first_index);
    }
    return try_close();
  }

  if(writer().is_closed() or writer().available_capacity() == 0){
    return;
  }

  const uint64_t unassemble_index {writer().bytes_pushed()};
  const uint64_t unacceptable_index {unassemble_index + writer().available_capacity()};

  if(first_index + data.size() <= unassemble_index
      or first_index >= unacceptable_index){
        return;
      }
  
  if(first_index + data.size() > unacceptable_index){
    is_last_substring = false;
    data.resize(unacceptable_index - first_index);
  }

  if(first_index < unassemble_index){
    data.erase(0,unassemble_index - first_index);
    first_index = unassemble_index;
  }

  if(not end_index_.has_value() and is_last_substring){
    end_index_.emplace(first_index + data.size());
  }

  const auto upper {split(first_index + data.size())};
  const auto lower {split(first_index)};

  ranges::for_each(ranges::subrange(lower,upper) | view::values,
                  [&](const auto& str) {total_pending_ -=str.size();});
  total_pending_ +=data.size();
  buf_.emplace_hint(buf_.erase(lower,upper),first_index,std::move(data));

  while(not buf_.empty()){
    auto&& [index,payload] {*buf_.begin()};
    if(index != writer().bytes_pushed()){
      break;
    }
    total_pending_ -= data.size();
    output_.writer().push(move(data));
    buf_.erase(buf_.begin());
  }
  return try_close();
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  return total_pending_;
}
