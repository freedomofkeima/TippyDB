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

typedef list<Data> Shard

struct ShardContent {
  1: Shard data
}

service DBService {

	void ping(),

	string putData(1:Data d),

	bool updateData(1:Data d),

	string getData(1:string sharded_key),

	bool deleteData(1:string sharded_key),

    /**
      * resyncData
      * Retrieve all newest shard contents where region = remote_region && node = remote_node
      */
    ShardContent resyncData(1:i32 remote_region, 2:i32 remote_node),

	oneway void zip()

}
