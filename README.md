# modern-cpp-json
A json library based on modern cpp.

## feature
* Single header file.
* Simple API.
* "Dynamic" type.

## How to use
You just need to download this header file and add it to your project.

## Example
#### Construct
```cpp
#include "json.hpp"

#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
	Json json =
	{
		{"int", 12306},
		{"double", 3.1415},
		{"string", "hello world"},
		{"vector", {1, 1, 2, 3, 5, 8, 11}},
		{"map",
		{
			{"key1", 123},
			{"key2", 0.321},
			{"key3", "balabala"},
			{"key4", {1, 2.0, "a"}}
		}}
	};

	cout << json << endl;

	return 0;
}
```

#### Serialization and deserialization
```cpp
auto str = dumps(json, 4);
```
```cpp
auto json = loads(str);
```

#### "Dynamic" type
```cpp
#include "json.hpp"

#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
	Json json = 123;

	json = {
		{"key", 3.14}
	};

	json["key"] = "value";

	return 0;
}
```
