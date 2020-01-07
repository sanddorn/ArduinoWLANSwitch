//
// Created by Nils Bokermann on 05.01.20.
//

#ifndef ARDUINOWLANSWITCH_FS_PERSISTENCE_H
#define ARDUINOWLANSWITCH_FS_PERSISTENCE_H
#ifndef UNIT_TEST

#include <utility>

#include "AbstractPersistence.h"
#include <FS.h>

class FileStorage : public StorageBlob {
public:
    explicit FileStorage(fs::File file) : actualStorage(std::move(file)) {};

    ~FileStorage() override = default;

    size_t size() const override;

    void close() override;

    bool isFile() const override;

    std::string readString() override;

private:
    fs::File actualStorage;

};

class FS_Persistence : public FilePersistence {
public:
    explicit FS_Persistence(fs::FS *filesystem) : fs(filesystem) {};

    ~FS_Persistence() override = default;

    bool begin() override;

    void end() override;

    std::shared_ptr<StorageBlob> open(const std::string path, const std::string mode) override;

private:
    fs::FS *fs;
};

#endif // UNIT_TEST
#endif //ARDUINOWLANSWITCH_FS_PERSISTENCE_H
