# kissearch
A fast fulltext search engine
###### Keep It Simple, Stupid
## Features
- BM25
- Cache
- Index
- Porter2 Stemmer (English)
- Tokenizer
- Load/Save index from file/memory
- Compression(when saving)
### Fields
- Number
- Text
- Keyword
- Boolean
## Lib
### Build/Install
```shell
cd lib && mkdir build && cd build && cmake .. && make && sudo make install
```
### CMake
```cmake
target_link_libraries(${PROJECT_NAME} /usr/local/lib/libkissearch.so)
```
### Example
```shell
example/
```
## Server
### Build
```shell
cd server && mkdir build && cd build && cmake .. && make
```
### Start
```shell
./server
```
### Use
```shell
curl -XPOST 0.0.0.0:8080/document/x -d '{"a":"text"}'
curl -XPOST 0.0.0.0:8080/document/x/add -d '{"a":"example"}'
curl -XPOST 0.0.0.0:8080/document/x/search -d '{"q":"example","field_names":"a"}'
```
## License
GNU Affero General Public License v3.0