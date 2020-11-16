#include <ResourceCollection.h>

#include <cstring>

namespace LOCC
{
    char* ResourceCollection::Lookup(char* key, char* buffer)
    {
        // This is result of reverse engineering of function at 0x00464FF0 aka ResourceCollection::Lookup
        char *keyWithoutLeadingSlash; // edx
        char currentChar; // al
        char currentCharInKeyWithoutLeadingSlash; // al
        size_t newIndex; // ecx
        int numChild; // ebx
        int keyIndex; // ebp
        int v8; // edi
        int v9; // eax
        int v10; // eax
        int v11; // esi
        int v12; // esi
        char *valuePtr; // edx
        size_t index; // [esp+10h] [ebp-Ch]
        int numChildOrg; // [esp+14h] [ebp-8h]
        char *pChunkName; // [esp+18h] [ebp-4h]

        while (true)
        {
            /// KEY NORMALISATION
            keyWithoutLeadingSlash = key;
            if ( *key == '/' )  /// Search place where our key starts not from /
            {
                do
                    currentChar = (keyWithoutLeadingSlash++)[1];
                while (currentChar == '/' );
                key = keyWithoutLeadingSlash;
            }

            currentCharInKeyWithoutLeadingSlash = *keyWithoutLeadingSlash;
            newIndex = 0;
            index = 0;

            if (*keyWithoutLeadingSlash != '/' )
            {
                do
                {
                    if ( !currentCharInKeyWithoutLeadingSlash ) // If we have zero terminator -> break
                        break;

                    currentCharInKeyWithoutLeadingSlash = keyWithoutLeadingSlash[newIndex++ + 1]; // save current char and increment newIndex
                }
                while (currentCharInKeyWithoutLeadingSlash != '/' ); // if our new char not slash -> continue

                index = newIndex;
            }

            /// KEY SEARCH AT THE CURRENT BRANCH
            numChild = (unsigned __int8)*buffer;
            keyIndex = 0;
            numChildOrg = (unsigned __int8)*buffer;
            if (numChild <= 0 )
                goto OnOrphanedTreeNodeDetected;

            pChunkName = &buffer[4 * numChild - 3];
            do
            {
                v8 = (numChild >> 1) + keyIndex;
                if ( v8 )
                    v9 = *(int *)&buffer[4 * v8 - 3];
                else
                    v9 = 0;

                int ret = 0;
                if ((ret = strnicmp(&pChunkName[v9], keyWithoutLeadingSlash, newIndex)) >= 0) // if value of first group greater or equal to our key
                {
                    numChild >>= 1; // Divide by two
                }
                else // Group name is less than our key
                {
                    keyIndex = v8 + 1;
                    numChild += -1 - (numChild >> 1);
                }

                newIndex = index;
                keyWithoutLeadingSlash = key;
            }
            while (numChild > 0);

            /// VALUE RESOLVING
            numChild = numChildOrg; //Restore back? o_0
            if ( keyIndex )
                v10 = *(int*)&buffer[4 * keyIndex - 3];
            else
                OnOrphanedTreeNodeDetected:
                v10 = 0;
            v11 = v10 + 4 * numChild - 3;

            int ret = 0;
            if (keyIndex >= numChild || (ret = strnicmp(&buffer[v11], keyWithoutLeadingSlash, newIndex)))
            {
                return nullptr;
            }

            /// ITERATION OVER TREE
            v12 = index + v11;
            valuePtr = &buffer[v12 + 2];
            buffer += v12 + 2;

            if ( !key[index] )
            {
                return valuePtr - 1;
            }

            key += index + 1;
        }
    }
}