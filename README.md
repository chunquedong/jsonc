# jsonc
Parse JSON text and compress to binary format.

20% smaller and 20% faster.

## JSON text parser benchmark

|  Name     |  Time     |  Codebase   |
|-----------|-----------|-------------|
| gason     |           |     650     |
| RapidJSON |           |  38,112     |
| jsonc     |           |     889 (jvalue.* + jparser.*)
| yyjson    |           |  18,000     |

## Compressed binary format

|  Format   |  Size     | Parse Time |
|-----------|-----------|------------|
| JSON text |   100%    |    100%    |
| version 1 |   20%     |     20%    |
| version 3 |   90%     |     0      |


## HiML parser
HiML is a JSON like data format for serialization.
```
//comment
name = form
Pane {
    x = 1
    y = 2
    Button {
        id = button1
        x=1
        y=2
    }
    Button {
        x = 1, y = 2
    }
    1,2,3,
    a
    b
    c
}
```
#### Key Idea
- no array, object as array
- no data type, metadata in mind
- quotes is options
- comma is options
- no braces for top level object
- comment by //


## Format (version 1)

| Type   |  ExtType |  Length(optional) |  Data(optional)    |
|--------|----------|-------------------|--------------------|
| 4bit   | 4bit     | 1/2/4/8byte       | length byte        | 

#### Primi
|  Name   | Type   |  ExtType |
|---------|--------|----------|
| null    | primi  | 0        |
| true    | primi  | 1        |
| false   | primi  | 2        |

#### Number
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

#### Array/Object
|  Name   | Type   |  ExtType        |  Length    |  Data         |
|---------|--------|-----------------|------------|---------------|
| array   | 3      | 0~10,12,13,14,15| 1/2/4/8byte| val1,val2,... |
| object  | 4      | 0~10,12,13,14,15| 1/2/4/8byte| key1,val1,key2,val2,... |

#### String
|  Name            | Type   |  ExtType        |  Length         |  Data      |
|------------------|--------|-----------------|-----------------|------------|
| new string       | 5      | 0~10,12,13,14,15| 1/2/4/8byte     | data       |

#### Ref to Pool
|  Name            | Type   |  ExtType        |  Index          |
|------------------|--------|-----------------|-----------------|
| ref              | 6      | 0~10,12,13,14,15| 1/2/4/8byte     |
