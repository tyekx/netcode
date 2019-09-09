function init(config)
    Printf("function called")
end

function update(dt) 
    rotation = GetRotation()
    rotation["x"] = 0;
    rotation["y"] = 1;
    rotation["z"] = 0;
    rotation["w"] = rotation["w"] + dt;
    SetRotation(rotation)
end
