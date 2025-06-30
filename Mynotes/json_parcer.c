bool parse_temperature(const char* json, float* temperature) {
    const char* ptr = json;
    
    // Skip whitespace
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
    
    // Check for opening brace
    if (*ptr++ != '{') return false;
    
    // Skip whitespace
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
    
    // Check for "temp" key
    if (strncmp(ptr, "\"temp\"", 6) != 0) return false;
    ptr += 6;
    
    // Skip whitespace
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
    
    // Check for colon
    if (*ptr++ != ':') return false;
    
    // Skip whitespace
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
    
    // Parse number
    float value = 0.0f;
    float sign = 1.0f;
    float decimal_place = 0.1f;
    bool is_decimal = false;
    
    // Handle sign
    if (*ptr == '-') {
        sign = -1.0f;
        ptr++;
    }
    
    // Parse digits
    while (1) {
        if (*ptr >= '0' && *ptr <= '9') {
            if (!is_decimal) {
                value = value * 10.0f + (*ptr - '0');
            } else {
                value += (*ptr - '0') * decimal_place;
                decimal_place *= 0.1f;
            }
            ptr++;
        } else if (*ptr == '.') {
            if (is_decimal) break; // Only one decimal point allowed
            is_decimal = true;
            ptr++;
        } else {
            break;
        }
    }
    
    *temperature = value * sign;
    
    // Skip whitespace
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
    
    // Check for closing brace
    if (*ptr++ != '}') return false;
    
    return true;
}
