#include "MergeFileContent.h"

MergeFileContent::MergeFileContent() = default;

bool MergeFileContent::isValid() const
{
    return !filePath.isEmpty();
}

bool MergeFileContent::hasAllVersions() const
{
    return !baseContent.isEmpty()
           || !oursContent.isEmpty()
           || !theirsContent.isEmpty();
}
