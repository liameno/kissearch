#!/bin/bash

curl -XPOST "0.0.0.0:8080/document/x" -d '{"id":"number", "title":"text", "url":"keyword"}'
curl -XPOST "0.0.0.0:8080/document/reserve" -d '{"size":7000}'

n=0
s=""

while [ $n -lt 7000 ]
do
  n=`expr $n + 1`
  s+="{ \"id\": \"${n}\", \"title\":\"The Hilltop algorithm is an algorithm used to find documents relevant to a particular keyword topic in news search\", \"url\":\"https://en.wikipedia.org/wiki/Hilltop_algorithm\" }
  ";
  n=`expr $n + 1`
  s+="{ \"id\": \"${n}\", \"title\":\"VisualRank is a system for finding and ranking images by analysing and comparing their content, rather than searching image names, Web links or other text\", \"url\":\"https://en.wikipedia.org/wiki/VisualRank\" }
  ";
  n=`expr $n + 1`
  s+="{ \"id\": \"${n}\", \"title\":\"TrustRank is an algorithm that conducts link analysis to separate useful webpages from spam and helps search engine rank pages in SERPs (Search Engine Results Pages)\", \"url\":\"https://en.wikipedia.org/wiki/TrustRank\" }
  ";
  n=`expr $n + 1`
  s+="{ \"id\": \"${n}\", \"title\": \"The CheiRank is an eigenvector with a maximal real eigenvalue of the Google matrix constructed for a directed network with the inverted directions of links\", \"url\": \"https://en.wikipedia.org/wiki/CheiRank\" }
  ";
  n=`expr $n + 1`
  s+="{ \"id\": \"${n}\", \"title\":\"PageRank (PR) is an algorithm used by Google Search to rank web pages in their search engine results\", \"url\":\"https://en.wikipedia.org/wiki/PageRank\" }
  ";
  n=`expr $n + 1`
  s+="{ \"id\": \"${n}\", \"title\":\"Okapi BM25 (BM is an abbreviation of best matching) is a ranking function used by search engines to estimate the relevance of documents to a given search query\", \"url\":\"https://en.wikipedia.org/wiki/Okapi_BM25\" }
  ";
  n=`expr $n + 1`
  s+="{ \"id\": \"${n}\", \"title\":\"term frequency–inverse document frequency\", \"url\":\"https://en.wikipedia.org/wiki/Tf%E2%80%93idf\" }
  ";
  curl -XPOST "0.0.0.0:8080/document/x/multi_add" -d"${s}" --silent > /dev/null
  s=""
  echo "${n}"
done

curl -XPOST "0.0.0.0:8080/document/x/index" -d ''
