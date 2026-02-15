cmake_minimum_required(VERSION 3.16)
project(TransactionCrypto LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)

add_library(TransactionCrypto STATIC
  RSA.cpp
  AES.cpp
  ECDSA.cpp
  Ultrasound.cpp
)
target_include_directories(TransactionCrypto PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(TransactionCrypto OpenSSL::SSL OpenSSL::Crypto)

add_executable(transaction_demo transaction.cpp)
target_link_libraries(transaction_demo PRIVATE TransactionCrypto)
