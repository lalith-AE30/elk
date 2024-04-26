import argparse


def generate_kernel(kernel_strength):
    return f"""float[](
        -{kernel_strength}, -{kernel_strength}, -{kernel_strength},
        -{kernel_strength},  {1 + 8*kernel_strength}, -{kernel_strength},
        -{kernel_strength}, -{kernel_strength}, -{kernel_strength}
    )"""


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--strength", type=float,
                        help="Strength of sharpening filter")
    parser.add_argument("--stride", type=lambda x: float(x) if float(x) >
                        0 else argparse.ArgumentTypeError("Value must be greater than 0"), help="Stride to sample texture")
    args = parser.parse_args()
    shader = None
    with open(r'shaders/Tscreen_sharpen.frag') as f:
        shader = f.read()

    with open(r'shaders/screen_sharpen.frag', 'w') as f:
        shader = shader.replace("KERNEL", generate_kernel(args.strength))
        shader = shader.replace("STRIDE", f'{args.stride}')
        f.write(shader)
