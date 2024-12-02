#ifndef CONFIG_SECRETS_H
#define CONFIG_SECRETS_H

struct ConfigSecrets
{
    const char *wifi_ssid = "<wifi_ssid>";
    const char *wifi_password = "<wifi_password>";
    const char *mqtt_broker = "<mqtt_broker>";
};

extern ConfigSecrets config_secrets;
#endif
