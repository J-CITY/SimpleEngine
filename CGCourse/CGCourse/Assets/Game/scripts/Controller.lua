local Controller =
{
    mouseSensitivity    = 0.2,
    mouseLook           = Vector2.new(180, 0),
    previousMouse       = Vector2.new(0, 0),
    firstMouse          = true,
 }

function Controller:OnUpdate(deltaTime)
    self:HandleMovement(deltaTime)
    self:HandleRotation(deltaTime)
end

function Controller:HandleMovement(deltaTime)
    pos = self.owner:GetTransform():GetLocalPosition()
    
    velocity = Vector3.new(0, 0, 0)
    forward = self.owner:GetTransform():GetForward()
    right   = self.owner:GetTransform():GetRight()

    if Inputs.GetKeyDown(Key.A) then velocity = velocity + right end
    if Inputs.GetKeyDown(Key.D) then velocity = velocity - right end
    if Inputs.GetKeyDown(Key.W) then velocity = velocity + forward end
    if Inputs.GetKeyDown(Key.S) then velocity = velocity - forward end
    pos = pos + velocity
    self.owner:GetTransform():SetLocalPosition(pos)
end

function Controller:HandleRotation(deltaTime)
    mousePosition = Inputs.GetMousePos()
    
    if self.firstMouse == true then
        self.previousMouse = mousePosition
        self.firstMouse = false
    end
    
    mouseOffset = Vector2.new(0, 0)
    mouseOffset.x = mousePosition.x - self.previousMouse.x
    mouseOffset.y = mousePosition.y - self.previousMouse.y

    self.previousMouse = mousePosition

    mouseOffset.x = mouseOffset.x * self.mouseSensitivity
    mouseOffset.y = mouseOffset.y * self.mouseSensitivity

    self.mouseLook = self.mouseLook + mouseOffset;

    if self.mouseLook.y > 89    then self.mouseLook.y = 89  end
    if self.mouseLook.y < -89   then self.mouseLook.y = -89 end

    self.owner:GetTransform():SetRotation(Quaternion.new(Vector3.new(self.mouseLook.y, -self.mouseLook.x, 0)))
end

return Controller