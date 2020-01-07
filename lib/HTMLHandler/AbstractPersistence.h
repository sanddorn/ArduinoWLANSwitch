//
// Created by Nils Bokermann on 05.01.20.
//

#ifndef ARDUINOWLANSWITCH_ABSTRACTPERSISTENCE_H
#define ARDUINOWLANSWITCH_ABSTRACTPERSISTENCE_H

#include <memory>
#include <string>

class StorageBlob {
public:
    virtual ~StorageBlob() = default;
    virtual size_t size() const = 0;

    virtual void close() = 0;

    virtual bool isFile() const = 0;

    virtual std::string readString() = 0;

};

class Persistence {
public:
    virtual ~Persistence() = default;

    virtual bool begin()  = 0;

    virtual void end() = 0;

    virtual std::shared_ptr<StorageBlob>  open(const std::string path, const std::string mode) = 0 ;
};
#endif //ARDUINOWLANSWITCH_ABSTRACTPERSISTENCE_H
