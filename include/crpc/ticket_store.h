#pragma once

class ticket_store {
public:
    ticket_store() : ticket_num_(0) {}

    void setValue(uint64_t ticket_number,
                  const cista::offset::vector<unsigned char>& payload)
    {
      auto ticket = tickets_.find(ticket_number);
      while (ticket == tickets_.end()) {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        ticket = tickets_.find(ticket_number);
      }
      ticket->second.set_value(std::vector<unsigned char>(
          payload.begin(), payload.end()));
      tickets_.erase(ticket_number);
    }

  uint64_t nextNumber()
  {
    return ++ticket_num_;
    }

    std::future<std::vector<unsigned char>> emplace(uint64_t ticket_number)
    {
      std::promise<std::vector<unsigned char>> promise;
      auto future = promise.get_future();
      while(!tickets_.emplace(ticket_number, std::move(promise)).second) {
        std::cerr << "full at size " << tickets_.size() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      return future;
    }
private:
  std::atomic<uint64_t> ticket_num_;
  cista::raw::hash_map<uint64_t, std::promise<std::vector<unsigned char>>> tickets_;
};