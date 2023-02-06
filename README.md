# 🧸 A toy language

Run examples with:

```
$ node toy.js examples/helloworld.toy
Hello, World!
```

# Benchmarks

| Language                         | File Size | Time   | Command              |
| -------------------------------- | --------- | ------ | -------------------- |
| Node (v18.9.0)                   | 4.3MB     | 130ms  | make benchmark-node  |
| Go (go1.18 darwin/arm64)         | 4.6MB     | 230ms  | make benchmark-go    |
| Perl (v5.30)                     | 3.8MB     | 260ms  | make benchmark-perl  |
| Clang (arm64-apple-darwin21.6.0) | 4.6MB     | 4500ms | make benchmark-clang |
