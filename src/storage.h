#ifndef STORAGE_H
    #define STORAGE_H

    class Storage
    {
        const char *data_folder         = "/data";
        const char *data_file           = "/data/data.txt";
        const char *logs_folder         = "/logs";
        const char *errors_file         = "/logs/errors.txt";
        const char *errors__file_backup = "/logs/errors_backup.txt";
        const uint32_t logs_file_limit  = 1024 * 32;

        public:
            void Setup();
            void End();
            void LogError(const char *message);
            void ConfigWrite(const uint16_t len, const uint8_t *state);
            bool ConfigRead(const uint16_t len, uint8_t *state);
            void ConfigDelete();
    };

    extern Storage storage;
#endif
