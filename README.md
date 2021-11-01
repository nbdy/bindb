# bindb

`binary file based database`

## features

- [X] insert
- [X] insertMultiple
- [X] find

## benchmark

### insert

`(Ryzen 9 3900xt) 12c / 24t @ 3.8GHz`

#### Single

| Count | Single |
| :---: | :----: |
| 1k    | 2s     |
| 10k   | 12s    |
| 100k  | 123s   |

#### Multiple

##### Chunks size: 10

| Count | Multiple |
| :---: | :------: |
| 1k    | 0s       |
| 10k   | 2s       |
| 100k  | 14s      |
| 1m    | 124s     |

##### Chunk size: 100

| Count | Multiple |
| :---: | :------: |
| 10k   | 0s       |
| 100k  | 1s       |
| 1m    | 14s      |

##### Chunk size: 1k

| Count | Multiple |
| :---: | :------: |
| 10k   | 0s       |
| 100k  | 0s       |
| 1m    | 2s       |

### find

#### Single

| Count | Single |
| :---: | :----: |
| 10k   | 0s     |
| 100k  | 0s     |
| 1m    | 17s    |

## size per entry

`sizeof(YourEntryType) + sizeof(size_t)`
