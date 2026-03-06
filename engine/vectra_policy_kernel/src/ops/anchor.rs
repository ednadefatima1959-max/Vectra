use crate::{arg_or_empty, parse_anchor_addr, DeterministicOp, Output};

pub(crate) struct AnchorOp;

pub(crate) static ANCHOR_OP: AnchorOp = AnchorOp;

impl DeterministicOp for AnchorOp {
    fn canonize(&self, args: &[String]) -> Vec<String> {
        vec![arg_or_empty(args, 0).trim().to_string()]
    }

    fn execute(&self, key_args: &[String]) -> Output {
        let anchor = parse_anchor_addr(arg_or_empty(key_args, 0));
        Output::Anchor(anchor)
    }

    fn op_code(&self) -> &'static str {
        "anchor"
    }
}
