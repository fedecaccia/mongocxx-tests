#include <iostream>
#include <typeinfo>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>


int main(int, char**) {

    mongocxx::instance inst{};
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);

    /* list databases */
    for(auto doc : client.list_databases()) {
        std::cout << bsoncxx::to_json(doc) << "\n";
    }

    /* access or create a database */
    mongocxx::database db = client["mydb"];
    //mongocxx::database db = client.database("mydb");

    /* access or create a collection */
    mongocxx::collection coll = db["mycoll"];
    //mongocxx::collection coll = db.collection("mycoll");

    /* create document */
    // bsoncxx::builder::stream::document document1 = bsoncxx::builder::stream::document{};
    // auto document1 = bsoncxx::builder::stream::document{};
    bsoncxx::builder::stream::document document1{};

    /* fill document:
    structure: 
    {
        "name" : "MongoDB",
        "type" : "database",
        "count" : 1,
        "versions": [ "v3.2", "v3.0", "v2.6" ],
        "info" : {
                    "x" : 203,
                    "y" : 102
                    }
    }
    */
    auto builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value doc_value = builder
    << "name" << "MongoDB"
    << "type" << "database"
    << "count" << 1
    << "versions" << bsoncxx::builder::stream::open_array
        << "v3.2" << "v3.0" << "v2.6"
    << bsoncxx::builder::stream::close_array
    << "info" << bsoncxx::builder::stream::open_document
        << "x" << 203
        << "y" << 102
    << bsoncxx::builder::stream::close_document
    << bsoncxx::builder::stream::finalize;


    /* This bsoncxx::document::value type is a read-only object owning its own memory.
    To use it, you must obtain a bsoncxx::document::view using the view() method: */
    bsoncxx::document::view view = doc_value.view();

    /* access view elements */
    bsoncxx::document::element element = view["name"];
    if(element.type() != bsoncxx::type::k_utf8) {
    // Error
    }
    std::string name = element.get_utf8().value.to_string();
    std::cout << "Name in document: " << name << std::endl;

    /*
    INSERT
    */

    /* insert one doc */
    // bsoncxx::stdx::optional<mongocxx::result::insert_one> result = coll.insert_one(document1);
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = coll.insert_one(document1.view());    
    if(result) {
        // std::cout << result->inserted_id() << "\n"; // HOW TO READ RESULT?
    }

    /* create multiple docs */
    std::vector<bsoncxx::document::value> documents;
    for(int i = 0; i < 100; i++) {
        documents.push_back(
        bsoncxx::builder::stream::document{} 
        // << "_id" << i*10
        << "i" << i 
        << bsoncxx::builder::stream::finalize);
    }

    /* insert many docs */
    bsoncxx::stdx::optional<mongocxx::result::insert_many> result2 = coll.insert_many(documents);
    // std::cout << "Generated ID in document: " << result2.inserted_ids() << std::endl;
    // std::string s = typeid(result2).name();
    // std::cout << "Result type: " << s << std::endl;
    if(result2) {
        std::cout << result2->inserted_count() << "\n";
    }
    
    /*
    QUERY
    */

    /* find one */
    bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result = coll.find_one({});
    if(maybe_result) {
        std::cout << bsoncxx::to_json(*maybe_result) << "\n";
    }

    /* find all */
    mongocxx::cursor cursor = coll.find({});
    for(auto doc : cursor) {
        std::cout << bsoncxx::to_json(doc) << "\n";
    }

    /* query one doc that match with filter */
    bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result2 = coll.find_one(
        bsoncxx::builder::stream::document{} 
        << "i" << 71 
        << bsoncxx::builder::stream::finalize
    );
    if(maybe_result2) {
        std::cout << bsoncxx::to_json(*maybe_result2) << "\n";
    }

    /* query multiple docs that match with filter */
    mongocxx::cursor cursor2 = coll.find(
        bsoncxx::builder::stream::document{}
            << "i" << bsoncxx::builder::stream::open_document
            << "$gt" << 50
            << "$lte" << 100
            << bsoncxx::builder::stream::close_document 
        << bsoncxx::builder::stream::finalize
    );
    for(auto doc : cursor2) {
        std::cout << bsoncxx::to_json(doc) << "\n";
    }

    /*
    UPDATE
    */

    /* update a single doc*/
    // result methods in /usr/local/include/mongocxx/v_noabi/mongocxx/result/update.hpp
    bsoncxx::stdx::optional<mongocxx::result::update> res_update = coll.update_one(
        bsoncxx::builder::stream::document{} 
            << "i" << 10 
            << bsoncxx::builder::stream::finalize,
        bsoncxx::builder::stream::document{} << "$set" 
            << bsoncxx::builder::stream::open_document <<
            "i" << 110 
            << bsoncxx::builder::stream::close_document 
        << bsoncxx::builder::stream::finalize
    );
    if(res_update) {
        std::cout << res_update->matched_count() << "\n";        
        std::cout << res_update->modified_count() << "\n";        
    }

    /* update multiple docs */
    bsoncxx::stdx::optional<mongocxx::result::update> result3 = coll.update_many(
        bsoncxx::builder::stream::document{} 
            << "i" << bsoncxx::builder::stream::open_document
                << "$lt" << 100 
                << bsoncxx::builder::stream::close_document 
            << bsoncxx::builder::stream::finalize,
        bsoncxx::builder::stream::document{} 
            << "$inc" << bsoncxx::builder::stream::open_document
                << "i" << 100 
                << bsoncxx::builder::stream::close_document 
            << bsoncxx::builder::stream::finalize
    );

    if(result3) {
        std::cout << result3->modified_count() << "\n";
    }

    /*
    DELETE
    */

    /* delete one doc */
    bsoncxx::stdx::optional<mongocxx::result::delete_result> result_del = coll.delete_one(
        bsoncxx::builder::stream::document{} 
        << "i" << 110 
        << bsoncxx::builder::stream::finalize
    );

    /* delete all docs that match a filter */
    bsoncxx::stdx::optional<mongocxx::result::delete_result> result_del2 =
    coll.delete_many(
        bsoncxx::builder::stream::document{}
        << "i" << bsoncxx::builder::stream::open_document
            << "$gte" << 100 
            << bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::finalize
    );

    if(result_del2) {
        std::cout << result_del2->deleted_count() << "\n";
    }

    /*
    INDEX
    */

    /* create an ascending index */
    auto index_specification = 
        bsoncxx::builder::stream::document{}
        << "i" << 1 // 1: ascending, -1:descending
        << bsoncxx::builder::stream::finalize;
    coll.create_index(std::move(index_specification));

}