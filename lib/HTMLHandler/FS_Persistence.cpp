//
// Created by Nils Bokermann on 05.01.20.
//

#include "FS_Persistence.h"

#ifndef UNIT_TEST

size_t FileStorage::size() const {
    return actualStorage.size();
}

void FileStorage::close() {
    actualStorage.close();

}

bool FileStorage::isFile() const {
    return actualStorage.isFile();
}

std::string FileStorage::readString() {
    return actualStorage.readString().c_str();
}

bool FS_Persistence::begin() {
    return fs->begin();
}

void FS_Persistence::end() {
    fs->end();
}

std::shared_ptr<StorageBlob> FS_Persistence::open(const char *path, const char *mode) {
    File file = fs->open(path, mode);
    std::shared_ptr<FileStorage> blob (new FileStorage(file));
    return blob;
}

#endif // UNIT_TEST
