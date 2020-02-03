#ifndef STORAGE_H
    #define STORAGE_H

    class Storage
    {
        public:
            void Setup();
            void LogError(const char *message);
            void ConfigWrite(const uint16_t len, const uint8_t *state);
            bool ConfigRead(const uint16_t len, uint8_t *state);
            void ConfigDelete();
    };

    extern Storage storage;
#endif
