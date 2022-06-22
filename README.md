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
```cpp
//example.cpp
int main() {
    const std::string file_name         = "../index.db";
    const std::string field_name_text   = "title";
    const std::string text_query        = "algorithms";

    document::search_options search_options_text;
    search_options_text.field_names = { field_name_text };
    
    document document;
    document.load(file_name);

    document.index_text_field(field_name_text);
    auto results = document.search(text_query, search_options_text);
    
    for (auto &result : results) {
        auto &field_id = result.first.find_field(field_name_number);
        auto &field = result.first.find_field(field_name_text);
        std::cout << field_id._number->value << " - " << field._text->value << " (score: " << result.second << ")" << std::endl;
    }

    document.save(file_name);
}
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