import argparse


def generate_blur(*args, **kwargs):
    return f"""float[](
        {1/16}, {2/16}, {1/16},
        {2/16}, {4/16}, {2/16},
        {1/16}, {2/16}, {1/16}
    )"""


def generate_sharpen(kernel_strength, detect_edges=False):
    return f"""float[](
        {-kernel_strength}, {-kernel_strength}, {-kernel_strength},
        {-kernel_strength}, { (not detect_edges) + 8*kernel_strength}, {-kernel_strength},
        {-kernel_strength}, {-kernel_strength}, {-kernel_strength}
    )"""


def generate_offsets(n):
    sym_n = int(n/2)
    syms = [f'-{i+1}*offset' for i in range(sym_n)][::-1]
    syms.append('0')
    syms.extend(f'{i+1}*offset' for i in range(sym_n))
    s = f'vec2 offsets[{n**2}] = vec2[]('
    return s + ','.join(f'\n\t\tvec2({x}, {y})' for y in syms[::-1] for x in syms) + '\n\t)'


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--type", type=str, default='sharpen',
                        choices=['sharpen', 'blur', 'edge'])
    parser.add_argument("--strength", type=float,
                        help="Strength of filter")
    parser.add_argument("--stride", default=1/300,
                        type=lambda x: float(x) if float(x) > 0 else argparse.ArgumentTypeError("Value must be greater than 0"), help="Stride to sample texture")
    args = parser.parse_args()
    shader = None
    with open(r'shaders/screen_generic.frag') as f:
        shader = f.read()

    with open(r'shaders/screen_postprocess.frag', 'w') as f:
        filter_fn = None
        detect_edges = False
        match args.type:
            case 'sharpen':
                filter_fn = generate_sharpen
            case 'blur':
                filter_fn = generate_blur
            case 'edge':
                filter_fn = generate_sharpen
                detect_edges = True

        shader = shader.replace("OFFSETS", generate_offsets(3))
        shader = shader.replace("OFFSET_SZ", '9')
        shader = shader.replace(
            "KERNEL", filter_fn(args.strength, detect_edges))
        shader = shader.replace("STRIDE", f'{args.stride}')
        f.write(shader)
