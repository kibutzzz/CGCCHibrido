# Roteiro — Apresentação Grau B

---

## Estrutura do projeto

**[Mostrar estrutura de arquivos]**

"O projeto é organizado em classes independentes em `Common/`. Vou passar por cada uma antes de rodar a aplicação."

---

## Classes

**[Abrir `Window.h`]**

"`Window` inicializa o GLFW, cria o contexto OpenGL 4.1 Core Profile e carrega os ponteiros de função via GLAD. Ela encapsula o loop de eventos — `pollEvents`, `swapBuffers`, `shouldClose`."

---

**[Abrir `ShaderProgram.h`]**

"`ShaderProgram` lê os arquivos GLSL do disco, compila vertex e fragment shader, linka o programa e expõe métodos para setar uniforms: `setMat4`, `setVec3`, `setFloat`."

---

**[Abrir `PhongShader.h`]**

"`PhongShader` estende `ShaderProgram` com métodos de domínio: `setMaterial` manda ka, kd, ks e ns para o shader; `setLights` itera sobre as luzes da cena multiplicando cor por intensidade; `setSelectionBrightness` controla o destaque do objeto selecionado."

---

**[Abrir `Mesh.h`]**

"`Mesh` tem três responsabilidades: parseia o `.obj` linha a linha montando um buffer plano de vértices com posição, normal e UV; lê o `.mtl` extraindo ka, kd, ks e ns; e carrega a textura com stb_image. No final, cria o VAO e VBO e configura os atributos com `glVertexAttribPointer` — stride de 11 floats: 3 posição, 3 cor, 3 normal, 2 UV."

---

**[Abrir `SceneObject.h`]**

"`SceneObject` é o objeto de cena: guarda o VAO, contagem de vértices, material, textura, posição, rotação, escala e a animação. O método `modelMatrix()` monta a transformação na ordem correta — translação, depois rotação em X, Y, Z, depois escala uniforme — e retorna a mat4 que vai para o vertex shader."

---

**[Abrir `AnimationPath.h`]**

"`AnimationPath` armazena os waypoints e interpola linearmente entre eles com `glm::mix`. O `update` avança o progresso proporcional ao deltaTime dividido pelo comprimento do segmento, mantendo velocidade constante independente da distância entre pontos. Ao chegar no último waypoint volta para o primeiro."

---

**[Abrir `Scene.h`]**

"`Scene` é o container da cena: lista de `SceneObject`, lista de `PointLight` e o índice do objeto selecionado. `selectNext` rotaciona a seleção e mantém a flag `selected` atualizada."

---

**[Abrir `Camera.h`]**

"`Camera` calcula a `viewMatrix` com `glm::lookAt` e a `projectionMatrix` com `glm::perspective`. O mouse atualiza yaw e pitch, que recalculam o vetor `front` via trigonometria. O pitch é limitado a ±89° para evitar gimbal lock."

---

**[Abrir `InputHandler.h`]**

"`InputHandler` implementa uma máquina de modos: Navigate, Transform, Animation e Light. Teclas 1, 2, 3 e Esc trocam de modo. Em cada modo, WASD tem semântica diferente — mover câmera, mover objeto, ou mover luz. Tab rotaciona a seleção. Space em modo animação adiciona um waypoint 10 unidades à frente da câmera."

---

**[Abrir `SceneLoader.h`]**

"`SceneLoader` lê o JSON com nlohmann/json: instancia câmera, luzes e objetos. Para cada objeto chama `loadSimpleOBJ` e `loadMTL`, aplica as transformações iniciais do JSON e, se houver campo `animation`, popula os waypoints do `AnimationPath`."

---

## Cena e shaders

**[Abrir `scene.json`]**

"A cena tem câmera com frustrum definido, três luzes com cor e intensidade independentes, dezesseis pedras de paralelepípedo formando o chão, casa, carro, avião e a bola com animação predefinida por waypoints. Tudo editável sem recompilar."

---

**[Abrir `vertex.vert`]**

"O vertex shader recebe posição, normal, UV e cor nos locations 0–3. Calcula a posição em espaço de mundo com a model matrix, passa para clip space com view e projection, e corrige a normal com a transposta inversa da model matrix — necessário para escala não uniforme."

---

**[Abrir `fragment.frag`]**

"O fragment shader acumula as contribuições de todas as luzes em um loop. Para cada luz calcula ambiente, difusa com `dot(normal, lightDir)` e especular com `reflect` e `pow`. O resultado é multiplicado pela textura e pelo `selectionBrightness`. Os coeficientes ka, kd, ks e ns vêm do `.mtl` lido em `Mesh`."

---

## Demo

**[Compilar e rodar]**

"Compilando e rodando."

---

**[Navegar, modo Transform — mostrar escala com +/-]**

"Modo navegar com mouse e WASD. Modo transformar: seleciono um objeto com Tab, movo, rotaciono e escalo com +/-."

---

**[Modo luz — ajustar intensidade]**

"Modo luz: repositiono e ajusto a intensidade de cada fonte com +/-. Dá pra ver a contribuição de cada cor em tempo real."

---

**[Modo animação — adicionar waypoints]**

"Modo animação: adiciono waypoints com Espaço. O objeto percorre o caminho em loop com velocidade constante."

---

**[Bola quicando ao fundo]**

"A bola tem animação predefinida no JSON — os waypoints simulam os saltos com altura decrescente."
