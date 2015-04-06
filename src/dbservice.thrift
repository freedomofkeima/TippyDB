/**
  * Iskandar Setiadi 13511073@std.stei.itb.ac.id
  * Institut Teknologi Bandung (ITB) - Indonesia
  * Final Project (c) 2015
  * dbservice.thrift
  *
  */

/**
 * Namespace definition
 */
namespace cpp dbservice
namespace php dbservice
namespace py dbservice

/**
 * Structs definition
 */
struct Data {
  1: string key,
  2: string value,
}

struct Exception {
  1: i32 error_code,
  2: string description
}

service DBService {

	void ping(),

    bool put(1:string key, 2:string value) throws (1:Exception exception),

    bool update(1:string key, 2:string value) throws (1:Exception exception),

    string get(1:string key) throws (1:Exception exception),

    bool delete(1:string key) throws (1:Exception exception),

	oneway void zip()

}
