# kissearch
A fast fulltext search engine
###### Keep It Simple, Stupid
## Features
- BM25
- Cache
- Index
- Porter2 Stemmer (English)
- Tokenizer
- Text/Keyword/Number fields
- Load/Save index from file/memory
- Compression(when saving)
## Build/Install
```shell
cd lib && mkdir build && cd build && cmake .. && make && sudo make install
```
## CMake
```cmake
target_link_libraries(${PROJECT_NAME} /usr/local/lib/libkissearch.so)
```
## Example
```shell
example/
```
## License
GNU Affero General Public License v3.0