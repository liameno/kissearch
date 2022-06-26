# kissearch

A fast fulltext search engine

###### Keep It Simple, Stupid

## Features

- BM25
- Cache
- Index
- Porter2 Stemmer
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
//full: example/example.cpp
int main() {
    const std::string file_name         = "../index.db";
    const std::string field_name_text   = "title";
    const std::string text_query        = "algorithms";
    
    document::search_options search_options_text;
    search_options_text.field_names = { field_name_text };

    collection collection;
    collection.documents.push_back(std::make_shared<document>());
    auto &document = *collection.documents.front();
    
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
GET /document/x #get document info 
# {
#   "entries":{"count":0},
#   "fields":[{"name":"a","type":"text"}],
#   "status":"ok"
# }
POST /document/x -d '{"a":"text"}' #create document 
# {
#   "status":"ok"
# }
POST /document/x/add -d '{"a":"example"}' #create entry 
# {
#   "status":"ok"
# }
POST document/x/index -d '' #index 
# {
#   "status":"ok"
# }
POST /document/x/remove -d '{"q":"example","field_names":"a"}' #remove entries, only after index
# {
#   "count":1,
#   "status":"ok"
# }
POST /document/x/search -d '{"q":"example","field_names":"a"}' #search entries, only after index
# {
#   "count":1,
#   "found":[{"entry":{"a":"example"},"score":0.2876820724517809}]
#   "status":"ok"
# }

# Errors("status":"error"):
#   exception "message":"*what*"
#   already_exists_document "message":"Already Exists"
#   not_found_document "message":"Not Found"
#   not_found_field "message":"Not Found Field"
```

```shell
curl -XGET 0.0.0.0:8080/document/x -d
curl -XPOST 0.0.0.0:8080/document/x -d '{"a":"text"}'
curl -XPOST 0.0.0.0:8080/document/x/add -d '{"a":"example"}'
curl -XPOST 0.0.0.0:8080/document/x/index -d ''
curl -XPOST 0.0.0.0:8080/document/x/remove -d '{"q":"example","field_names":"a"}'
curl -XPOST 0.0.0.0:8080/document/x/search -d '{"q":"example","field_names":"a"}'
```

## Comparison

```shell
comparison/
```

| Name              | Memory | Time(index/add) | Time(search) |
|-------------------|--------|-----------------|--------------|
| Kissearch(server) | 30mb   | 12s             | 9ms          |
| Kissearch(lib)    | 28mb   | 0.2s            | 0.7ms        |
| Typesense(server) | 125mb  | 53s             | 13ms         |

###### Entries Size: 7000

### Kissearch: 30 mb
#### Typesense: 118 mb

## License

GNU Affero General Public License v3.0