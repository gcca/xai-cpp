# xai-cpp

xai-cpp es una biblioteca cliente en C++ para interactuar con la API de x.ai. Proporciona una interfaz simple y moderna en C++ para enviar mensajes a modelos de inteligencia artificial y recibir respuestas, con soporte tanto para respuestas directas como para streaming en tiempo real.

## Características

- **API fácil de usar** para enviar mensajes y recibir respuestas.
- Soporte para **listar modelos disponibles y modelos de lenguaje**.
- **REPL (Read-Eval-Print Loop)** para sesiones interactivas con visualización de respuestas en tiempo real mediante streaming.
- Construido con **C++23** y sigue altos estándares de calidad de código con advertencias estrictas del compilador.

## Requisitos

- Un compilador compatible con **C++23** (por ejemplo, GCC 13, Clang 16).
- **CMake 3.30 o superior**.
- Bibliotecas **Boost** (componentes: system, thread, json).
- Biblioteca **OpenSSL**.

## Construcción del Proyecto

Para construir el proyecto, sigue estos pasos:

1. Clona el repositorio:
   ```
   git clone https://github.com/tu_usuario/xai-cpp.git
   cd xai-cpp
   ```

2. Crea un directorio de construcción:
   ```
   mkdir build
   cd build
   ```

3. Ejecuta CMake:
   ```
   cmake ..
   ```

4. Compila el proyecto:
   ```
   cmake --build .
   ```

5. Opcionalmente, habilita las pruebas configurando la opción `XAI_ENABLE_TESTS`:
   ```
   cmake .. -DXAI_ENABLE_TESTS=ON
   cmake --build .
   ```

## Uso

### REPL

El proyecto incluye un ejecutable `xai-repl` para sesiones interactivas:

1. Ejecuta el programa con tu clave API:
   ```
   ./xai-repl <tu_clave_api>
   ```

2. En la sesión interactiva, escribe mensajes para enviarlos al modelo de IA. Las respuestas se mostrarán en tiempo real mediante streaming:
   - Escribe `\q` para salir.
   - Escribe `\m` para listar los modelos disponibles.
   - Escribe `\l` para listar los modelos de lenguaje disponibles.

   **Nota:** Por defecto, el REPL usa el modelo `"grok-beta"`. Asegúrate de que tu clave API tenga acceso a este modelo o modifica el código en `xai-repl.cc` para usar otro modelo si es necesario.

### Usando la Biblioteca

Para usar la biblioteca en tu propio código, incluye el archivo de cabecera `xai.hpp` y utiliza las clases `xai::Client` y `xai::Messages`.

#### Ejemplo en Modo Directo

```
#include "xai.hpp"
#include <iostream>

int main() {
    auto client = xai::Client::Make("tu_clave_api");
    auto messages = xai::Messages::Make("grok-beta");
    messages->AddU("¡Hola, IA!");
    auto choices = client->ChatCompletion(messages);
    std::cout << choices->first() << std::endl;
    return 0;
}
```

#### Ejemplo en Modo Streaming

```
#include "xai.hpp"
#include <iostream>
#include <sstream>

int main() {
    auto client = xai::Client::Make("tu_clave_api");
    auto messages = xai::Messages::Make("grok-beta");
    messages->AddU("¡Hola, IA!");
    std::ostringstream oss;
    client->ChatCompletion(messages, [&](std::unique_ptr<xai::Choices> choices) {
        auto part = choices->first();
        oss << part;
        std::cout << part;
    });
    std::cout << std::endl;
    // oss.str() contiene la respuesta completa
    return 0;
}
```

#### Listado de Modelos

```
auto model_list = client->ListModels();
model_list->Traverse([](const xai::ModelList::Model &model) {
    std::cout << model.id << std::endl;
});

auto lang_model_list = client->ListLanguageModels();
lang_model_list->Traverse([](const xai::LanguageModelList::LanguageModel &model) {
    std::cout << model.id << std::endl;
});
```

**Nota:** Reemplaza `"grok-beta"` con un nombre de modelo válido de la API de x.ai según tu acceso.

## Pruebas

Si habilitaste las pruebas durante la construcción, puedes ejecutarlas con:

```
ctest
```

O directamente con el ejecutable:

```
./xai-test
```

## Usando la Biblioteca en Tu Proyecto

Para integrar xai-cpp en tu propio proyecto CMake:

1. Agrega el directorio fuente de xai-cpp a tu proyecto (por ejemplo, mediante un submódulo de Git o copiando el código fuente).

2. En tu `CMakeLists.txt`, añade:

   ```
   add_subdirectory(xai-cpp)
   target_link_libraries(tu_objetivo xAI::xAI)
   ```

3. Incluye los encabezados en tu código:

   ```
   #include "xai.hpp"
   ```

Luego, utiliza las clases como se muestra en los ejemplos anteriores.

## Licencia

Este proyecto está licenciado bajo los términos del archivo [LICENSE](LICENSE).

## Dependencias

- [Boost](https://www.boost.org/)
- [OpenSSL](https://www.openssl.org/)
- [Google Test](https://github.com/google/googletest)
