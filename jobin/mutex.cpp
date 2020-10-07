#include"mutex.hpp"

mutex::mutex() {}
mutex::~mutex() { mut.~mutex(); }

void mutex::lock() { mut.lock(); }
void mutex::unlock() { mut.unlock(); }