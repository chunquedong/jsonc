# jsonc
Compress JSON to binary format.

55% smaller and 32% faster.

# Format

| Type   |  ExtType |  Length(optional) |  Data(optional)    |
|--------|----------|-------------------|--------------------|
| 4bit   | 4bit     | 1/2/4/8byte       | length byte        | 

### Primi
|  Name   | Type   |  ExtType |
|---------|--------|----------|
| null    | primi  | 0        |
| true    | primi  | 1        |
| false   | primi  | 2        |

### Number
|  Name       | Type   |  ExtType |  Length   |
|-------------|--------|----------|-----------|
| tiny int    | 1      | 0~10,11  |           |
| int8        | 1      | 12       |  1byte    |
| int16       | 1      | 13       |  2byte    |
| int32       | 1      | 14       |  4byte    |
| int64       | 1      | 15       |  8byte    |
| tiny float  | 2      | 0~10,11  |           |
| float32     | 2      | 14       |  4byte    |
| float64     | 2      | 15       |  8byte    |

## Array/Object
|  Name   | Type   |  ExtType        |  Length    |  Data         |
|---------|--------|-----------------|------------|---------------|
| array   | 3      | 0~10,12,13,14,15| 1/2/4/8byte| val1,val2,... |
| object  | 4      | 0~10,12,13,14,15| 1/2/4/8byte| key1,val1,key2,val2,... |

## String
|  Name            | Type   |  ExtType        |  Length/Index   |  Data      |
|------------------|--------|-----------------|-----------------|------------|
| new string       | 5      | 0~10,12,13,14,15| 1/2/4/8byte     | data       |
| string by index  | 6      | 0~10,12,13,14,15| 1/2/4/8byte     |            |

