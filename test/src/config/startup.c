// SPDX-License-Identifier: LGPL-2.1-or-later
#include "../../libvlc/test.h"
#include "../../../lib/libvlc_internal.h"
#include <vlc_configuration.h>
#include <vlc_fs.h>
#include <vlc_variables.h>
#include <vlc_modules.h>

struct persistent_options
{
    unsigned count;
    bool rate, volume, device, instance;
};

static void persistent_option(void *opaque, const char *name)
{
    struct persistent_options *options = opaque;
    assert(config_IsPersistent(name));
    assert(strcmp(name, "config") && strcmp(name, "help")
        && strcmp(name, "load-startup-options"));
    ++options->count;
    options->rate |= !strcmp(name, "rate");
    options->volume |= !strcmp(name, "audio-last-volume");
    options->device |= !strcmp(name, "audio-device-default-module");
    options->instance |= !strcmp(name, "single-instance-preferences-version");
}

static void expansion(const char *text, const char *const expected[], size_t count)
{
    char *setting;
    assert(asprintf(&setting, "--startup-options=%s", text) >= 0);
    const char *args[] = {"--load-startup-options", setting, "--rate=2"};
    char **expanded = NULL;
    int argc = config_ExpandStartupOptions(ARRAY_SIZE(args), args, &expanded);
    assert(argc == (int)(count + ARRAY_SIZE(args)));
    if (count == 0) assert(expanded == NULL);
    else
    {
        assert(expanded != NULL);
        for (size_t i = 0; i < count; ++i) assert(!strcmp(expanded[i], expected[i]));
        for (size_t i = 0; i < ARRAY_SIZE(args); ++i) assert(!strcmp(expanded[count+i], args[i]));
        assert(expanded[argc] == NULL);
    }
    config_FreeStartupOptions(expanded);
    free(setting);
}

static void test_split(void)
{
    expansion("", NULL, 0);
    expansion(" \t ", NULL, 0);
    const char *basic[] = {"--no-audio", "-vv", "--rate=1.25"};
    expansion("--no-audio -vv --rate=1.25", basic, ARRAY_SIZE(basic));
    const char *quotes[] = {"--http-user-agent=Player test", "--http-pwd", "", "--snapshot-path=C:\\My Videos", "--input-title-format=a'b"};
    expansion("--http-user-agent=\"Player test\" --http-pwd '' --snapshot-path=\"C:\\My Videos\" --input-title-format=\"a'b\"", quotes, ARRAY_SIZE(quotes));
    const char *network[] = {"--snapshot-path=\\\\NAS\\Movies\\", "--input-title-format=a\"b", "--http-user-agent=semi;$(literal)`text`"};
    expansion("--snapshot-path=\"\\\\NAS\\Movies\\\\\" --input-title-format=\"a\\\"b\" --http-user-agent='semi;$(literal)`text`'", network, ARRAY_SIZE(network));
    const char *flags[] = {"--load-startup-options", "--startup-options=\"unclosed"};
    char **result = NULL;
    assert(config_ExpandStartupOptions(ARRAY_SIZE(flags), flags, &result) < 0);
    assert(result == NULL);
    const char *terminator[] = {"--load-startup-options", "--startup-options=--rate=2 -- --no-video"};
    assert(config_ExpandStartupOptions(ARRAY_SIZE(terminator), terminator, &result) < 0);
    assert(result == NULL);
}

static void test_bypass(void)
{
    const char *base[] = {"--load-startup-options", "--startup-options=--no-audio", "--ignore-config"};
    char **result = NULL;
    assert(config_ExpandStartupOptions(ARRAY_SIZE(base), base, &result) == ARRAY_SIZE(base));
    assert(result == NULL);
    const char *reset[] = {"--load-startup-options", "--startup-options=--no-audio", "--reset-config"};
    assert(config_ExpandStartupOptions(ARRAY_SIZE(reset), reset, &result) == ARRAY_SIZE(reset));
    assert(result == NULL);
    const char *embed[] = {"--startup-options=--no-audio"};
    assert(config_ExpandStartupOptions(ARRAY_SIZE(embed), embed, &result) == ARRAY_SIZE(embed));
    assert(result == NULL);
    const char *off[] = {"--load-startup-options", "--startup-options=--no-audio", "--no-load-startup-options"};
    assert(config_ExpandStartupOptions(ARRAY_SIZE(off), off, &result) == ARRAY_SIZE(off));
    assert(result == NULL);
    const char *end[] = {"--startup-options=--no-audio", "--", "--load-startup-options"};
    assert(config_ExpandStartupOptions(ARRAY_SIZE(end), end, &result) == ARRAY_SIZE(end));
    assert(result == NULL);
}

static libvlc_instance_t *instance(const char *config, const char *option, bool standalone)
{
    const char *args[] = {"--no-media-library", "--no-metadata-network-access",
        "--no-ignore-config", config, option,
        standalone ? "--load-startup-options" : "--no-load-startup-options"};
    return libvlc_new(ARRAY_SIZE(args), args);
}

static void test_runtime(void)
{
    char filename[] = "startup-options-XXXXXX";
    int fd = vlc_mkstemp(filename);
    assert(fd != -1);
    FILE *file = fdopen(fd, "w");
    assert(file != NULL);
    fputs("\xef\xbb\xbf[core]\n#startup-options=--rate=9\nstartup-options=--no-audio --rate=1.25\n", file);
    fclose(file);
    char *config;
    assert(asprintf(&config, "--config=%s", filename) >= 0);
    libvlc_instance_t *base = instance(config, "--no-video", true);
    assert(base != NULL);
    assert(!var_InheritBool(base->p_libvlc_int, "audio"));
    assert(!var_InheritBool(base->p_libvlc_int, "video"));
    assert(var_InheritFloat(base->p_libvlc_int, "rate") == 1.25f);
    assert(config_IsPersistent("rate"));
    assert(config_IsPersistent("startup-options"));
    assert(config_FindConfig("config") != NULL);
    assert(!config_IsPersistent("config"));
    assert(!config_IsPersistent("help"));
    assert(!config_IsPersistent("not-a-real-vlc-setting"));
    assert(!config_IsPersistent(NULL));
    /* A live plugin definition takes priority over an obsolete alias with
     * the same name. SRT input removed payload-size; SRT output still uses it. */
    size_t module_count;
    module_t **modules = module_list_get(&module_count);
    for (size_t i = 0; i < module_count; ++i)
    {
        unsigned count;
        module_config_t *items = module_config_get(modules[i], &count);
        for (unsigned j = 0; j < count; ++j)
            if (items[j].psz_name && !strcmp(items[j].psz_name, "payload-size"))
            {
                assert(config_IsPersistent("payload-size"));
                assert(config_FindConfig("payload-size")->orig.i == items[j].orig.i);
            }
        module_config_free(items);
    }
    module_list_free(modules);
    struct persistent_options persistent = {0};
    config_ForEachPersistent(persistent_option, &persistent);
    assert(persistent.count > 0 && persistent.rate && persistent.volume
        && persistent.device && persistent.instance);
    vlc_object_t *object = VLC_OBJECT(base->p_libvlc_int);
    assert(config_ValidateStartupOptions(object, "--rate=3 --video --http-user-agent='Sample player'") == VLC_SUCCESS);
    assert(var_InheritFloat(base->p_libvlc_int, "rate") == 1.25f);
    assert(!var_InheritBool(base->p_libvlc_int, "video"));
    assert(config_ValidateStartupOptions(object, "--rate") != VLC_SUCCESS);
    assert(config_ValidateStartupOptions(object, "--not-a-real-vlc-setting") != VLC_SUCCESS);
    assert(config_ValidateStartupOptions(object, "--rate='unclosed") != VLC_SUCCESS);
    assert(config_ValidateStartupOptions(object, "--rate=1\n--no-audio") != VLC_SUCCESS);
    assert(config_ValidateStartupOptions(object, "movie.mkv") != VLC_SUCCESS);
    assert(config_ValidateStartupOptions(object, "-- --no-audio") != VLC_SUCCESS);

    libvlc_instance_t *other = instance(config, "--rate=2", true);
    assert(other != NULL);
    assert(var_InheritFloat(other->p_libvlc_int, "rate") == 2.f);
    libvlc_release(other);
    other = instance(config, "--ignore-config", true);
    assert(other != NULL);
    assert(var_InheritBool(other->p_libvlc_int, "audio"));
    libvlc_release(other);
    other = instance(config, "--no-video", false);
    assert(other != NULL);
    assert(var_InheritBool(other->p_libvlc_int, "audio"));
    libvlc_release(other);
    libvlc_release(base);

    file = vlc_fopen(filename, "w");
    assert(file != NULL);
    fputs("[core]\nstartup-options=--removed-plugin-option\n", file);
    fclose(file);
    other = instance(config, "--rate=2", true);
    assert(other != NULL); /* Invalid saved options cannot lock out the app. */
    assert(var_InheritFloat(other->p_libvlc_int, "rate") == 2.f);
    libvlc_release(other);
    other = instance(config, "--invalid-real-launch-option", true);
    assert(other == NULL); /* A real launch error is still reported. */
    assert(vlc_unlink(filename) == 0);
    free(config);
}

int main(void)
{
    test_init();
    test_split();
    test_bypass();
    test_runtime();
    return 0;
}
