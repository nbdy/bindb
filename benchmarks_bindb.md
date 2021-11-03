# Benchmarks
## `(Ryzen 9 3900xt) 12c / 24t @ 3.8GHz`
### insert

#### Single

| Count | Seconds | Kilobytes |
| :---: | :-----: | :-------: |
| 1k    | 2       | 80        |
| 10k   | 12      | 800       |
| 100k  | 123     | 8000      |

#### Multiple

##### Chunks size: 10

| Count | Seconds |
| :---: | :-----: |
| 1k    | 0       |
| 10k   | 2       |
| 100k  | 14      |
| 1m    | 124     |

##### Chunk size: 100

| Count | Seconds |
| :---: | :-----: |
| 10k   | 0       |
| 100k  | 1       |
| 1m    | 14      |

##### Chunk size: 1k

| Count | Seconds |
| :---: | :-----: |
| 10k   | 0       |
| 100k  | 0       |
| 1m    | 2       |

### find

| Count | Seconds |
| :---: | :-----: |
| 10k   | 0       |
| 100k  | 1       |
| 1m    | 17      |