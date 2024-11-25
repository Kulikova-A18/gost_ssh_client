#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
typedef struct
{
    pthread_mutex_t check_thread_mutex;
    char *control_path;
    char *cipher;
} MySSHClient;
void x2goclient_clear_strings(gpointer data)
{
    g_free(data);
}
void ssh_connect(MySSHClient *self, const char *hostname, const char *username, const char *message)
{
    g_mutex_lock(&(self->check_thread_mutex));

    GPtrArray *ssh_cmd = g_ptr_array_new_with_free_func(&x2goclient_clear_strings);
    g_ptr_array_add(ssh_cmd, g_strdup("ssh"));
    g_ptr_array_add(ssh_cmd, g_strdup("-o"));
    g_ptr_array_add(ssh_cmd, g_strdup_printf("ControlPath=%s", self->control_path));
    if (self->cipher)
    {
        g_ptr_array_add(ssh_cmd, g_strdup("-o"));
        g_ptr_array_add(ssh_cmd, g_strdup_printf("Ciphers=%s", self->cipher));
    }
    else
    {
        g_ptr_array_add(ssh_cmd, g_strdup("-o"));
        g_ptr_array_add(ssh_cmd, g_strdup("Ciphers=3des-cbc,aes128-cbc,aes192-cbc,aes256-cbc,aes128-ctr,aes192-ctr,aes256-ctr,aes128-gcm@openssh.com,aes256-gcm@openssh.com,chacha20-poly1305@openssh.com,grasshopper-cbc@altlinux.org,grasshopper-ctr@altlinux.org,magma-cbc@altlinux.org,magma-ctr@altlinux.org"));
    }
    g_ptr_array_add(ssh_cmd, g_strdup("-o"));
    g_ptr_array_add(ssh_cmd, g_strdup("KexAlgorithms=diffie-hellman-group1-sha1,diffie-hellman-group14-sha1,diffie-hellman-group14-sha256,diffie-hellman-group16-sha512,diffie-hellman-group18-sha512,diffie-hellman-group-exchange-sha1,diffie-hellman-group-exchange-sha256,ecdh-sha2-nistp256,ecdh-sha2-nistp384,ecdh-sha2-nistp521,curve25519-sha256,curve25519-sha256@libssh.org,sntrup761x25519-sha512@openssh.com"));
    g_ptr_array_add(ssh_cmd, g_strdup_printf("%s@%s", username, hostname));
    char *command = g_strdup_printf(
        "echo " % s " > /tmp/tmp_client_certificate.crt && "
                    "openssl verify -CAfile /etc/ssl/certs/ca_certificates_x2go.cert.pem /tmp/tmp_client_certificate.crt && "
                    "{ CN=$(openssl x509 -in /tmp/tmptmp_client_certificate_cert.crt -noout -subject | awk -F 'CN =' '{print $2}' | awk '{print $1}'); "
                    "if getent passwd | awk -F: '{print $1}' | grep -q " ^
            $CN$ "; then "
                 "echo "Успех
                 "; else echo "Не найдено "; rm /tmp/tmp_client_certificate.crt; fi; } || "
                 "{ echo "Ошибка в проверке сертификата "; rm /tmp/tmp_client_certificate.crt; }",
        message);
    g_ptr_array_add(ssh_cmd, g_strdup("bash"));
    g_ptr_array_add(ssh_cmd, command);
    char **argv = (char **)g_malloc((ssh_cmd->len + 1) * sizeof(char *));
    for (guint i = 0; i < ssh_cmd->len; i++)
    {
        argv[i] = (char *)g_ptr_array_index(ssh_cmd, i);
    }
    argv[ssh_cmd->len] = NULL;
    if (execvp(argv[0], argv) == -1)
    {
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }
    for (guint i = 0; i < ssh_cmd->len; i++)
    {
        g_free(argv[i]);
    }
    g_free(argv);
    g_mutex_unlock(&(self->check_thread_mutex));
}
char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("can't open file");
        return NULL;
    }
    size_t size = 0;
    size_t capacity = 128;
    char *buffer = malloc(capacity);
    if (buffer == NULL)
    {
        perror("can't read file (=null)");
        fclose(file);
        return NULL;
    }
    int ch;
    while ((ch = fgetc(file)) != EOF)
    {
        if (size + 1 >= capacity)
        {
            capacity *= 2;
            char *new_buffer = realloc(buffer, capacity);
            if (new_buffer == NULL)
            {
                perror("can't read file (=null)");
                free(buffer);
                fclose(file);
                return NULL;
            }
            buffer = new_buffer;
        }
        buffer[size++] = ch;
    }

    buffer[size] = '0';
    fclose(file);
    return buffer;
}
int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        fprintf(stderr, "Usage: %s <hostname> <username> <cipher> <control_path> <path/to/cert>n", argv[0]);
        return EXIT_FAILURE;
    }

    MySSHClient client;
    pthread_mutex_init(&(client.check_thread_mutex), NULL);
    client.control_path = argv[4];
    client.cipher = argv[3];

    char *content = read_file(argv[5]);

    if (content != NULL)
    {
        ssh_connect(&client, argv[1], argv[2], content);
        free(content);
    }

    pthread_mutex_destroy(&(client.check_thread_mutex));
    return EXIT_SUCCESS;
}
