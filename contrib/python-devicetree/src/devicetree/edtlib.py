# Copyright (c) 2019 Nordic Semiconductor ASA
# Copyright (c) 2019 Linaro Limited
# SPDX-License-Identifier: BSD-3-Clause

# Tip: You can view just the documentation with 'pydoc3 devicetree.edtlib'

"""
Library for working with devicetrees at a higher level compared to dtlib. Like
dtlib, this library presents a tree of devicetree nodes, but the nodes are
augmented with information from bindings and include some interpretation of
properties. Some of this interpretation is based on conventions established
by the Linux kernel, so the Documentation/devicetree/bindings in the Linux
source code is sometimes good reference material.

Bindings are YAML files that describe devicetree nodes. Devicetree
nodes are usually mapped to bindings via their 'compatible = "..."' property,
but a binding can also come from a 'child-binding:' key in the binding for the
parent devicetree node.

Each devicetree node (dtlib.Node) gets a corresponding edtlib.Node instance,
which has all the information related to the node.

The top-level entry points for the library are the EDT and Binding classes.
See their constructor docstrings for details. There is also a
bindings_from_paths() helper function.
"""

# NOTE: tests/test_edtlib.py is the test suite for this library.

# Implementation notes
# --------------------
#
# A '_' prefix on an identifier in Python is a convention for marking it private.
# Please do not access private things. Instead, think of what API you need, and
# add it.
#
# This module is not meant to have any global state. It should be possible to
# create several EDT objects with independent binding paths and flags. If you
# need to add a configuration parameter or the like, store it in the EDT
# instance, and initialize it e.g. with a constructor argument.
#
# This library is layered on top of dtlib, and is not meant to expose it to
# clients. This keeps the header generation script simple.
#
# General biased advice:
#
# - Consider using @property for APIs that don't need parameters. It makes
#   functions look like attributes, which is less awkward in clients, and makes
#   it easy to switch back and forth between variables and functions.
#
# - Think about the data type of the thing you're exposing. Exposing something
#   as e.g. a list or a dictionary is often nicer and more flexible than adding
#   a function.
#
# - Avoid get_*() prefixes on functions. Name them after the thing they return
#   instead. This often makes the code read more naturally in callers.
#
#   Also, consider using @property instead of get_*().
#
# - Don't expose dtlib stuff directly.
#
# - Add documentation for any new APIs you add.
#
#   The convention here is that docstrings (quoted strings) are used for public
#   APIs, and "doc comments" for internal functions.
#
#   @properties are documented in the class docstring, as if they were
#   variables. See the existing @properties for a template.

from collections import OrderedDict, defaultdict
from copy import deepcopy
import logging
import os
import re
from typing import Set

import yaml
try:
    # Use the C LibYAML parser if available, rather than the Python parser.
    # This makes e.g. gen_defines.py more than twice as fast.
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader     # type: ignore

from devicetree.dtlib import DT, DTError, to_num, to_nums, Type
from devicetree.grutils import Graph


#
# Public classes
#


class EDT:
    """
    Represents a devicetree augmented with information from bindings.

    These attributes are available on EDT objects:

    nodes:
      A list of Node objects for the nodes that appear in the devicetree

    compat2nodes:
      A collections.defaultdict that maps each 'compatible' string that appears
      on some Node to a list of Nodes with that compatible.

    compat2okay:
      Like compat2nodes, but just for nodes with status 'okay'.

    label2node:
      A collections.OrderedDict that maps a node label to the node with
      that label.

    dep_ord2node:
      A collections.OrderedDict that maps an ordinal to the node with
      that dependency ordinal.

    chosen_nodes:
      A collections.OrderedDict that maps the properties defined on the
      devicetree's /chosen node to their values. 'chosen' is indexed by
      property name (a string), and values are converted to Node objects.
      Note that properties of the /chosen node which can't be converted
      to a Node are not included in the value.

    dts_path:
      The .dts path passed to __init__()

    dts_source:
      The final DTS source code of the loaded devicetree after merging nodes
      and processing /delete-node/ and /delete-property/, as a string

    bindings_dirs:
      The bindings directory paths passed to __init__()

    scc_order:
      A list of lists of Nodes. All elements of each list
      depend on each other, and the Nodes in any list do not depend
      on any Node in a subsequent list. Each list defines a Strongly
      Connected Component (SCC) of the graph.

      For an acyclic graph each list will be a singleton. Cycles
      will be represented by lists with multiple nodes. Cycles are
      not expected to be present in devicetree graphs.

    The standard library's pickle module can be used to marshal and
    unmarshal EDT objects.
    """
    def __init__(self, dts, bindings_dirs,
                 warn_reg_unit_address_mismatch=True,
                 default_prop_types=True,
                 support_fixed_partitions_on_any_bus=True,
                 infer_binding_for_paths=None,
                 vendor_prefixes=None,
                 werror=False):
        """EDT constructor.

        dts:
          Path to devicetree .dts file

        bindings_dirs:
          List of paths to directories containing bindings, in YAML format.
          These directories are recursively searched for .yaml files.

        warn_reg_unit_address_mismatch (default: True):
          If True, a warning is logged if a node has a 'reg' property where
          the address of the first entry does not match the unit address of the
          node

        default_prop_types (default: True):
          If True, default property types will be used when a node has no
          bindings.

        support_fixed_partitions_on_any_bus (default True):
          If True, set the Node.bus for 'fixed-partitions' compatible nodes
          to None.  This allows 'fixed-partitions' binding to match regardless
          of the bus the 'fixed-partition' is under.

        infer_binding_for_paths (default: None):
          An iterable of devicetree paths identifying nodes for which bindings
          should be inferred from the node content.  (Child nodes are not
          processed.)  Pass none if no nodes should support inferred bindings.

        vendor_prefixes (default: None):
          A dict mapping vendor prefixes in compatible properties to their
          descriptions. If given, compatibles in the form "manufacturer,device"
          for which "manufacturer" is neither a key in the dict nor a specially
          exempt set of grandfathered-in cases will cause warnings.

        werror (default: False):
          If True, some edtlib specific warnings become errors. This currently
          errors out if 'dts' has any deprecated properties set, or an unknown
          vendor prefix is used.
        """
        self._warn_reg_unit_address_mismatch = warn_reg_unit_address_mismatch
        self._default_prop_types = default_prop_types
        self._fixed_partitions_no_bus = support_fixed_partitions_on_any_bus
        self._infer_binding_for_paths = set(infer_binding_for_paths or [])
        self._werror = bool(werror)
        self._vendor_prefixes = vendor_prefixes

        self.dts_path = dts
        self.bindings_dirs = bindings_dirs

        self._dt = DT(dts)
        _check_dt(self._dt)

        self._init_compat2binding()
        self._init_nodes()
        self._init_graph()
        self._init_luts()

        self._check()

    def get_node(self, path):
        """
        Returns the Node at the DT path or alias 'path'. Raises EDTError if the
        path or alias doesn't exist.
        """
        try:
            return self._node2enode[self._dt.get_node(path)]
        except DTError as e:
            _err(e)

    @property
    def chosen_nodes(self):
        ret = OrderedDict()

        try:
            chosen = self._dt.get_node("/chosen")
        except DTError:
            return ret

        for name, prop in chosen.props.items():
            try:
                node = prop.to_path()
            except DTError:
                # DTS value is not phandle or string, or path doesn't exist
                continue

            ret[name] = self._node2enode[node]

        return ret

    def chosen_node(self, name):
        """
        Returns the Node pointed at by the property named 'name' in /chosen, or
        None if the property is missing
        """
        return self.chosen_nodes.get(name)

    @property
    def dts_source(self):
        return f"{self._dt}"

    def __repr__(self):
        return "<EDT for '{}', binding directories '{}'>".format(
            self.dts_path, self.bindings_dirs)

    @property
    def scc_order(self):
        try:
            return self._graph.scc_order()
        except Exception as e:
            raise EDTError(e)

    def _init_graph(self):
        # Constructs a graph of dependencies between Node instances,
        # which is usable for computing a partial order over the dependencies.
        # The algorithm supports detecting dependency loops.
        #
        # Actually computing the SCC order is lazily deferred to the
        # first time the scc_order property is read.

        self._graph = Graph()

        for node in self.nodes:
            # A Node always depends on its parent.
            for child in node.children.values():
                self._graph.add_edge(child, node)

            # A Node depends on any Nodes present in 'phandle',
            # 'phandles', or 'phandle-array' property values.
            for prop in node.props.values():
                if prop.type == 'phandle':
                    self._graph.add_edge(node, prop.val)
                elif prop.type == 'phandles':
                    for phandle_node in prop.val:
                        self._graph.add_edge(node, phandle_node)
                elif prop.type == 'phandle-array':
                    for cd in prop.val:
                        if cd is None:
                            continue
                        self._graph.add_edge(node, cd.controller)

            # A Node depends on whatever supports the interrupts it
            # generates.
            for intr in node.interrupts:
                self._graph.add_edge(node, intr.controller)

    def _init_compat2binding(self):
        # Creates self._compat2binding, a dictionary that maps
        # (<compatible>, <bus>) tuples (both strings) to Binding objects.
        #
        # The Binding objects are created from YAML files discovered
        # in self.bindings_dirs as needed.
        #
        # For example, self._compat2binding["company,dev", "can"]
        # contains the Binding for the 'company,dev' device, when it
        # appears on the CAN bus.
        #
        # For bindings that don't specify a bus, <bus> is None, so that e.g.
        # self._compat2binding["company,notonbus", None] is the Binding.
        #
        # Only bindings for 'compatible' strings that appear in the devicetree
        # are loaded.

        dt_compats = _dt_compats(self._dt)
        # Searches for any 'compatible' string mentioned in the devicetree
        # files, with a regex
        dt_compats_search = re.compile(
            "|".join(re.escape(compat) for compat in dt_compats)
        ).search

        self._binding_paths = _binding_paths(self.bindings_dirs)
        self._binding_fname2path = {os.path.basename(path): path
                                    for path in self._binding_paths}

        self._compat2binding = {}
        for binding_path in self._binding_paths:
            with open(binding_path, encoding="utf-8") as f:
                contents = f.read()

            # As an optimization, skip parsing files that don't contain any of
            # the .dts 'compatible' strings, which should be reasonably safe
            if not dt_compats_search(contents):
                continue

            # Load the binding and check that it actually matches one of the
            # compatibles. Might get false positives above due to comments and
            # stuff.

            try:
                # Parsed PyYAML output (Python lists/dictionaries/strings/etc.,
                # representing the file)
                raw = yaml.load(contents, Loader=_BindingLoader)
            except yaml.YAMLError as e:
                _err(
                        f"'{binding_path}' appears in binding directories "
                        f"but isn't valid YAML: {e}")
                continue

            # Convert the raw data to a Binding object, erroring out
            # if necessary.
            binding = self._binding(raw, binding_path, dt_compats)

            # Register the binding in self._compat2binding, along with
            # any child bindings that have their own compatibles.
            while binding is not None:
                if binding.compatible:
                    self._register_binding(binding)
                binding = binding.child_binding

    def _binding(self, raw, binding_path, dt_compats):
        # Convert a 'raw' binding from YAML to a Binding object and return it.
        #
        # Error out if the raw data looks like an invalid binding.
        #
        # Return None if the file doesn't contain a binding or the
        # binding's compatible isn't in dt_compats.

        # Get the 'compatible:' string.
        if raw is None or "compatible" not in raw:
            # Empty file, binding fragment, spurious file, etc.
            return None

        compatible = raw["compatible"]

        if compatible not in dt_compats:
            # Not a compatible we care about.
            return None

        # Initialize and return the Binding object.
        return Binding(binding_path, self._binding_fname2path, raw=raw)

    def _register_binding(self, binding):
        # Do not allow two different bindings to have the same
        # 'compatible:'/'on-bus:' combo
        old_binding = self._compat2binding.get((binding.compatible,
                                                binding.on_bus))
        if old_binding:
            msg = (f"both {old_binding.path} and {binding.path} have "
                   f"'compatible: {binding.compatible}'")
            if binding.on_bus is not None:
                msg += f" and 'on-bus: {binding.on_bus}'"
            _err(msg)

        # Register the binding.
        self._compat2binding[binding.compatible, binding.on_bus] = binding

    def _init_nodes(self):
        # Creates a list of edtlib.Node objects from the dtlib.Node objects, in
        # self.nodes

        # Maps each dtlib.Node to its corresponding edtlib.Node
        self._node2enode = {}

        self.nodes = []

        for dt_node in self._dt.node_iter():
            # Warning: We depend on parent Nodes being created before their
            # children. This is guaranteed by node_iter().
            node = Node()
            node.edt = self
            node._node = dt_node
            if "compatible" in node._node.props:
                node.compats = node._node.props["compatible"].to_strings()
            else:
                node.compats = []
            node.bus_node = node._bus_node(self._fixed_partitions_no_bus)
            node._init_binding()
            node._init_regs()

            self.nodes.append(node)
            self._node2enode[dt_node] = node

        for node in self.nodes:
            # These depend on all Node objects having been created, because
            # they (either always or sometimes) reference other nodes, so we
            # run them separately
            node._init_props(default_prop_types=self._default_prop_types,
                             err_on_deprecated=self._werror)
            node._init_interrupts()
            node._init_pinctrls()

        if self._warn_reg_unit_address_mismatch:
            # This warning matches the simple_bus_reg warning in dtc
            for node in self.nodes:
                if node.regs and node.regs[0].addr != node.unit_addr:
                    _LOG.warning("unit address and first address in 'reg' "
                                 f"(0x{node.regs[0].addr:x}) don't match for "
                                 f"{node.path}")

    def _init_luts(self):
        # Initialize node lookup tables (LUTs).

        self.label2node = OrderedDict()
        self.dep_ord2node = OrderedDict()
        self.compat2nodes = defaultdict(list)
        self.compat2okay = defaultdict(list)

        for node in self.nodes:
            for label in node.labels:
                self.label2node[label] = node

            for compat in node.compats:
                self.compat2nodes[compat].append(node)

                if node.status == "okay":
                    self.compat2okay[compat].append(node)

        for nodeset in self.scc_order:
            node = nodeset[0]
            self.dep_ord2node[node.dep_ordinal] = node

    def _check(self):
        # Tree-wide checks and warnings.

        for binding in self._compat2binding.values():
            for spec in binding.prop2specs.values():
                if not spec.enum or spec.type != 'string':
                    continue

                if not spec.enum_tokenizable:
                    _LOG.warning(
                        f"compatible '{binding.compatible}' "
                        f"in binding '{binding.path}' has non-tokenizable enum "
                        f"for property '{spec.name}': " +
                        ', '.join(repr(x) for x in spec.enum))
                elif not spec.enum_upper_tokenizable:
                    _LOG.warning(
                        f"compatible '{binding.compatible}' "
                        f"in binding '{binding.path}' has enum for property "
                        f"'{spec.name}' that is only tokenizable "
                        'in lowercase: ' +
                        ', '.join(repr(x) for x in spec.enum))

        # Validate the contents of compatible properties.
        self._checked_compatibles: Set[str] = set()
        for node in self.nodes:
            if 'compatible' not in node.props:
                continue

            compatibles = node.props['compatible'].val

            # _check() runs after _init_compat2binding() has called
            # _dt_compats(), which already converted every compatible
            # property to a list of strings. So we know 'compatibles'
            # is a list, but add an assert for future-proofing.
            assert isinstance(compatibles, list)

            for compat in compatibles:
                # This is also just for future-proofing.
                assert isinstance(compat, str)

                self._check_compatible(node, compat)
        del self._checked_compatibles  # We have no need for this anymore.

    def _check_compatible(self, node, compat):
        if compat in self._checked_compatibles:
            return

        # The regular expression comes from dt-schema.
        compat_re = r'^[a-zA-Z][a-zA-Z0-9,+\-._]+$'
        if not re.match(compat_re, compat):
            _err(f"node '{node.path}' compatible '{compat}' "
                 'must match this regular expression: '
                 f"'{compat_re}'")

        if ',' in compat and self._vendor_prefixes is not None:
            vendor = compat.split(',', 1)[0]
            if vendor not in self._vendor_prefixes and \
                   vendor not in _VENDOR_PREFIX_ALLOWED:
                if self._werror:
                    log_fn = _LOG.error
                else:
                    log_fn = _LOG.warning
                log_fn(
                    f"node '{node.path}' compatible '{compat}' "
                    f"has unknown vendor prefix '{vendor}'")

        self._checked_compatibles.add(compat)

class Node:
    """
    Represents a devicetree node, augmented with information from bindings, and
    with some interpretation of devicetree properties. There's a one-to-one
    correspondence between devicetree nodes and Nodes.

    These attributes are available on Node objects:

    edt:
      The EDT instance this node is from

    name:
      The name of the node

    unit_addr:
      An integer with the ...@<unit-address> portion of the node name,
      translated through any 'ranges' properties on parent nodes, or None if
      the node name has no unit-address portion

    description:
      The description string from the binding for the node, or None if the node
      has no binding. Leading and trailing whitespace (including newlines) is
      removed.

    path:
      The devicetree path of the node

    label:
      The text from the 'label' property on the node, or None if the node has
      no 'label'

    labels:
      A list of all of the devicetree labels for the node, in the same order
      as the labels appear, but with duplicates removed.

      This corresponds to the actual devicetree source labels, unlike the
      "label" attribute, which is the value of a devicetree property named
      "label".

    parent:
      The Node instance for the devicetree parent of the Node, or None if the
      node is the root node

    children:
      A dictionary with the Node instances for the devicetree children of the
      node, indexed by name

    dep_ordinal:
      A non-negative integer value such that the value for a Node is
      less than the value for all Nodes that depend on it.

      The ordinal is defined for all Nodes, and is unique among nodes in its
      EDT 'nodes' list.

    required_by:
      A list with the nodes that directly depend on the node

    depends_on:
      A list with the nodes that the node directly depends on

    status:
      The node's status property value, as a string, or "okay" if the node
      has no status property set. If the node's status property is "ok",
      it is converted to "okay" for consistency.

    read_only:
      True if the node has a 'read-only' property, and False otherwise

    matching_compat:
      The 'compatible' string for the binding that matched the node, or None if
      the node has no binding

    binding_path:
      The path to the binding file for the node, or None if the node has no
      binding

    compats:
      A list of 'compatible' strings for the node, in the same order that
      they're listed in the .dts file

    regs:
      A list of Register objects for the node's registers

    props:
      A collections.OrderedDict that maps property names to Property objects.
      Property objects are created for all devicetree properties on the node
      that are mentioned in 'properties:' in the binding.

    aliases:
      A list of aliases for the node. This is fetched from the /aliases node.

    interrupts:
      A list of ControllerAndData objects for the interrupts generated by the
      node. The list is empty if the node does not generate interrupts.

    pinctrls:
      A list of PinCtrl objects for the pinctrl-<index> properties on the
      node, sorted by index. The list is empty if the node does not have any
      pinctrl-<index> properties.

    bus:
      If the node is a bus node (has a 'bus:' key in its binding), then this
      attribute holds the bus type, e.g. "i2c" or "spi". If the node is not a
      bus node, then this attribute is None.

    on_bus:
      The bus the node appears on, e.g. "i2c" or "spi". The bus is determined
      by searching upwards for a parent node whose binding has a 'bus:' key,
      returning the value of the first 'bus:' key found. If none of the node's
      parents has a 'bus:' key, this attribute is None.

    bus_node:
      Like on_bus, but contains the Node for the bus controller, or None if the
      node is not on a bus.

    flash_controller:
      The flash controller for the node. Only meaningful for nodes representing
      flash partitions.

    spi_cs_gpio:
      The device's SPI GPIO chip select as a ControllerAndData instance, if it
      exists, and None otherwise. See
      Documentation/devicetree/bindings/spi/spi-controller.yaml in the Linux kernel.
    """
    @property
    def name(self):
        "See the class docstring"
        return self._node.name

    @property
    def unit_addr(self):
        "See the class docstring"

        # TODO: Return a plain string here later, like dtlib.Node.unit_addr?

        if "@" not in self.name:
            return None

        try:
            addr = int(self.name.split("@", 1)[1], 16)
        except ValueError:
            _err("{!r} has non-hex unit address".format(self))

        return _translate(addr, self._node)

    @property
    def description(self):
        "See the class docstring."
        if self._binding:
            return self._binding.description
        return None

    @property
    def path(self):
        "See the class docstring"
        return self._node.path

    @property
    def label(self):
        "See the class docstring"
        if "label" in self._node.props:
            return self._node.props["label"].to_string()
        return None

    @property
    def labels(self):
        "See the class docstring"
        return self._node.labels

    @property
    def parent(self):
        "See the class docstring"
        return self.edt._node2enode.get(self._node.parent)

    @property
    def children(self):
        "See the class docstring"
        # Could be initialized statically too to preserve identity, but not
        # sure if needed. Parent nodes being initialized before their children
        # would need to be kept in mind.
        return OrderedDict((name, self.edt._node2enode[node])
                           for name, node in self._node.nodes.items())

    @property
    def required_by(self):
        "See the class docstring"
        return self.edt._graph.required_by(self)

    @property
    def depends_on(self):
        "See the class docstring"
        return self.edt._graph.depends_on(self)

    @property
    def status(self):
        "See the class docstring"
        status = self._node.props.get("status")

        if status is None:
            as_string = "okay"
        else:
            as_string = status.to_string()

        if as_string == "ok":
            as_string = "okay"

        return as_string

    @property
    def read_only(self):
        "See the class docstring"
        return "read-only" in self._node.props

    @property
    def aliases(self):
        "See the class docstring"
        return [alias for alias, node in self._node.dt.alias2node.items()
                if node is self._node]

    @property
    def bus(self):
        "See the class docstring"
        if self._binding:
            return self._binding.bus
        return None

    @property
    def on_bus(self):
        "See the class docstring"
        bus_node = self.bus_node
        return bus_node.bus if bus_node else None

    @property
    def flash_controller(self):
        "See the class docstring"

        # The node path might be something like
        # /flash-controller@4001E000/flash@0/partitions/partition@fc000. We go
        # up two levels to get the flash and check its compat. The flash
        # controller might be the flash itself (for cases like NOR flashes).
        # For the case of 'soc-nv-flash', we assume the controller is the
        # parent of the flash node.

        if not self.parent or not self.parent.parent:
            _err("flash partition {!r} lacks parent or grandparent node"
                 .format(self))

        controller = self.parent.parent
        if controller.matching_compat == "soc-nv-flash":
            return controller.parent
        return controller

    @property
    def spi_cs_gpio(self):
        "See the class docstring"

        if not (self.on_bus == "spi" and "cs-gpios" in self.bus_node.props):
            return None

        if not self.regs:
            _err("{!r} needs a 'reg' property, to look up the chip select index "
                 "for SPI".format(self))

        parent_cs_lst = self.bus_node.props["cs-gpios"].val

        # cs-gpios is indexed by the unit address
        cs_index = self.regs[0].addr
        if cs_index >= len(parent_cs_lst):
            _err("index from 'regs' in {!r} ({}) is >= number of cs-gpios "
                 "in {!r} ({})".format(
                     self, cs_index, self.bus_node, len(parent_cs_lst)))

        return parent_cs_lst[cs_index]

    def __repr__(self):
        return "<Node {} in '{}', {}>".format(
            self.path, self.edt.dts_path,
            "binding " + self.binding_path if self.binding_path
                else "no binding")

    def _init_binding(self):
        # Initializes Node.matching_compat, Node._binding, and
        # Node.binding_path.
        #
        # Node._binding holds the data from the node's binding file, in the
        # format returned by PyYAML (plain Python lists, dicts, etc.), or None
        # if the node has no binding.

        # This relies on the parent of the node having already been
        # initialized, which is guaranteed by going through the nodes in
        # node_iter() order.

        if self.path in self.edt._infer_binding_for_paths:
            self._binding_from_properties()
            return

        if self.compats:
            on_bus = self.on_bus

            for compat in self.compats:
                # When matching, respect the order of the 'compatible' entries,
                # and for each one first try to match against an explicitly
                # specified bus (if any) and then against any bus. This is so
                # that matching against bindings which do not specify a bus
                # works the same way in Zephyr as it does elsewhere.
                if (compat, on_bus) in self.edt._compat2binding:
                    binding = self.edt._compat2binding[compat, on_bus]
                elif (compat, None) in self.edt._compat2binding:
                    binding = self.edt._compat2binding[compat, None]
                else:
                    continue

                self.binding_path = binding.path
                self.matching_compat = compat
                self._binding = binding
                return
        else:
            # No 'compatible' property. See if the parent binding has
            # a compatible. This can come from one or more levels of
            # nesting with 'child-binding:'.

            binding_from_parent = self._binding_from_parent()
            if binding_from_parent:
                self._binding = binding_from_parent
                self.binding_path = self._binding.path
                self.matching_compat = self._binding.compatible

                return

        # No binding found
        self._binding = self.binding_path = self.matching_compat = None

    def _binding_from_properties(self):
        # Sets up a Binding object synthesized from the properties in the node.

        if self.compats:
            _err(f"compatible in node with inferred binding: {self.path}")

        # Synthesize a 'raw' binding as if it had been parsed from YAML.
        raw = {
            'description': 'Inferred binding from properties, via edtlib.',
            'properties': {},
        }
        for name, prop in self._node.props.items():
            pp = OrderedDict()
            if prop.type == Type.EMPTY:
                pp["type"] = "boolean"
            elif prop.type == Type.BYTES:
                pp["type"] = "uint8-array"
            elif prop.type == Type.NUM:
                pp["type"] = "int"
            elif prop.type == Type.NUMS:
                pp["type"] = "array"
            elif prop.type == Type.STRING:
                pp["type"] = "string"
            elif prop.type == Type.STRINGS:
                pp["type"] = "string-array"
            elif prop.type == Type.PHANDLE:
                pp["type"] = "phandle"
            elif prop.type == Type.PHANDLES:
                pp["type"] = "phandles"
            elif prop.type == Type.PHANDLES_AND_NUMS:
                pp["type"] = "phandle-array"
            elif prop.type == Type.PATH:
                pp["type"] = "path"
            else:
                _err(f"cannot infer binding from property: {prop} "
                     f"with type {prop.type!r}")
            raw['properties'][name] = pp

        # Set up Node state.
        self.binding_path = None
        self.matching_compat = None
        self.compats = []
        self._binding = Binding(None, {}, raw=raw, require_compatible=False)

    def _binding_from_parent(self):
        # Returns the binding from 'child-binding:' in the parent node's
        # binding.

        if not self.parent:
            return None

        pbinding = self.parent._binding
        if not pbinding:
            return None

        if pbinding.child_binding:
            return pbinding.child_binding

        return None

    def _bus_node(self, support_fixed_partitions_on_any_bus = True):
        # Returns the value for self.bus_node. Relies on parent nodes being
        # initialized before their children.

        if not self.parent:
            # This is the root node
            return None

        # Treat 'fixed-partitions' as if they are not on any bus.  The reason is
        # that flash nodes might be on a SPI or controller or SoC bus.  Having
        # bus be None means we'll always match the binding for fixed-partitions
        # also this means want processing the fixed-partitions node we wouldn't
        # try to do anything bus specific with it.
        if support_fixed_partitions_on_any_bus and "fixed-partitions" in self.compats:
            return None

        if self.parent.bus:
            # The parent node is a bus node
            return self.parent

        # Same bus node as parent (possibly None)
        return self.parent.bus_node

    def _init_props(self, default_prop_types=False, err_on_deprecated=False):
        # Creates self.props. See the class docstring. Also checks that all
        # properties on the node are declared in its binding.

        self.props = OrderedDict()

        node = self._node
        if self._binding:
            prop2specs = self._binding.prop2specs
        else:
            prop2specs = None

        # Initialize self.props
        if prop2specs:
            for prop_spec in prop2specs.values():
                self._init_prop(prop_spec, err_on_deprecated)
            self._check_undeclared_props()
        elif default_prop_types:
            for name in node.props:
                if name not in _DEFAULT_PROP_SPECS:
                    continue
                prop_spec = _DEFAULT_PROP_SPECS[name]
                val = self._prop_val(name, prop_spec.type, False, False, None,
                                     err_on_deprecated)
                self.props[name] = Property(prop_spec, val, self)

    def _init_prop(self, prop_spec, err_on_deprecated):
        # _init_props() helper for initializing a single property.
        # 'prop_spec' is a PropertySpec object from the node's binding.

        name = prop_spec.name
        prop_type = prop_spec.type
        if not prop_type:
            _err("'{}' in {} lacks 'type'".format(name, self.binding_path))

        val = self._prop_val(name, prop_type, prop_spec.deprecated,
                             prop_spec.required, prop_spec.default,
                             err_on_deprecated)

        if val is None:
            # 'required: false' property that wasn't there, or a property type
            # for which we store no data.
            return

        enum = prop_spec.enum
        if enum and val not in enum:
            _err("value of property '{}' on {} in {} ({!r}) is not in 'enum' "
                 "list in {} ({!r})"
                 .format(name, self.path, self.edt.dts_path, val,
                         self.binding_path, enum))

        const = prop_spec.const
        if const is not None and val != const:
            _err("value of property '{}' on {} in {} ({!r}) is different from "
                 "the 'const' value specified in {} ({!r})"
                 .format(name, self.path, self.edt.dts_path, val,
                         self.binding_path, const))

        # Skip properties that start with '#', like '#size-cells', and mapping
        # properties like 'gpio-map'/'interrupt-map'
        if name[0] == "#" or name.endswith("-map"):
            return

        self.props[name] = Property(prop_spec, val, self)

    def _prop_val(self, name, prop_type, deprecated, required, default,
                  err_on_deprecated):
        # _init_prop() helper for getting the property's value
        #
        # name:
        #   Property name from binding
        #
        # prop_type:
        #   Property type from binding (a string like "int")
        #
        # deprecated:
        #   True if the property is deprecated
        #
        # required:
        #   True if the property is required to exist
        #
        # default:
        #   Default value to use when the property doesn't exist, or None if
        #   the binding doesn't give a default value
        #
        # err_on_deprecated:
        #   If True, a deprecated property is an error instead of warning.

        node = self._node
        prop = node.props.get(name)

        if prop and deprecated:
            msg = (f"'{name}' is marked as deprecated in 'properties:' "
                   f"in {self.binding_path} for node {node.path}.")
            if err_on_deprecated:
                _err(msg)
            else:
                _LOG.warning(msg)

        if not prop:
            if required and self.status == "okay":
                _err("'{}' is marked as required in 'properties:' in {}, but "
                     "does not appear in {!r}".format(
                         name, self.binding_path, node))

            if default is not None:
                # YAML doesn't have a native format for byte arrays. We need to
                # convert those from an array like [0x12, 0x34, ...]. The
                # format has already been checked in
                # _check_prop_type_and_default().
                if prop_type == "uint8-array":
                    return bytes(default)
                return default

            return False if prop_type == "boolean" else None

        if prop_type == "boolean":
            if prop.type != Type.EMPTY:
                _err("'{0}' in {1!r} is defined with 'type: boolean' in {2}, "
                     "but is assigned a value ('{3}') instead of being empty "
                     "('{0};')".format(name, node, self.binding_path, prop))
            return True

        if prop_type == "int":
            return prop.to_num()

        if prop_type == "array":
            return prop.to_nums()

        if prop_type == "uint8-array":
            return prop.to_bytes()

        if prop_type == "string":
            return prop.to_string()

        if prop_type == "string-array":
            return prop.to_strings()

        if prop_type == "phandle":
            return self.edt._node2enode[prop.to_node()]

        if prop_type == "phandles":
            return [self.edt._node2enode[node] for node in prop.to_nodes()]

        if prop_type == "phandle-array":
            # This type is a bit high-level for dtlib as it involves
            # information from bindings and *-names properties, so there's no
            # to_phandle_array() in dtlib. Do the type check ourselves.
            if prop.type not in (Type.PHANDLE, Type.PHANDLES, Type.PHANDLES_AND_NUMS):
                _err(f"expected property '{name}' in {node.path} in "
                     f"{node.dt.filename} to be assigned "
                     f"with '{name} = < &foo ... &bar 1 ... &baz 2 3 >' "
                     f"(a mix of phandles and numbers), not '{prop}'")

            return self._standard_phandle_val_list(prop)

        if prop_type == "path":
            return self.edt._node2enode[prop.to_path()]

        # prop_type == "compound". Checking that the 'type:'
        # value is valid is done in _check_prop_type_and_default().
        #
        # 'compound' is a dummy type for properties that don't fit any of the
        # patterns above, so that we can require all entries in 'properties:'
        # to have a 'type: ...'. No Property object is created for it.
        return None

    def _check_undeclared_props(self):
        # Checks that all properties are declared in the binding

        for prop_name in self._node.props:
            # Allow a few special properties to not be declared in the binding
            if prop_name.endswith("-controller") or \
               prop_name.startswith("#") or \
               prop_name.startswith("pinctrl-") or \
               prop_name in {
                   "compatible", "status", "ranges", "phandle",
                   "interrupt-parent", "interrupts-extended", "device_type"}:
                continue

            if prop_name not in self._binding.prop2specs:
                _err("'{}' appears in {} in {}, but is not declared in "
                     "'properties:' in {}"
                     .format(prop_name, self._node.path, self.edt.dts_path,
                             self.binding_path))

    def _init_regs(self):
        # Initializes self.regs

        node = self._node

        self.regs = []

        if "reg" not in node.props:
            return

        address_cells = _address_cells(node)
        size_cells = _size_cells(node)

        for raw_reg in _slice(node, "reg", 4*(address_cells + size_cells),
                              "4*(<#address-cells> (= {}) + <#size-cells> (= {}))"
                              .format(address_cells, size_cells)):
            reg = Register()
            reg.node = self
            if address_cells == 0:
                reg.addr = None
            else:
                reg.addr = _translate(to_num(raw_reg[:4*address_cells]), node)
            if size_cells == 0:
                reg.size = None
            else:
                reg.size = to_num(raw_reg[4*address_cells:])
            if size_cells != 0 and reg.size == 0:
                _err("zero-sized 'reg' in {!r} seems meaningless (maybe you "
                     "want a size of one or #size-cells = 0 instead)"
                     .format(self._node))

            self.regs.append(reg)

        _add_names(node, "reg", self.regs)

    def _init_pinctrls(self):
        # Initializes self.pinctrls from any pinctrl-<index> properties

        node = self._node

        # pinctrl-<index> properties
        pinctrl_props = [prop for name, prop in node.props.items()
                         if re.match("pinctrl-[0-9]+", name)]
        # Sort by index
        pinctrl_props.sort(key=lambda prop: prop.name)

        # Check indices
        for i, prop in enumerate(pinctrl_props):
            if prop.name != "pinctrl-" + str(i):
                _err("missing 'pinctrl-{}' property on {!r} - indices should "
                     "be contiguous and start from zero".format(i, node))

        self.pinctrls = []
        for prop in pinctrl_props:
            pinctrl = PinCtrl()
            pinctrl.node = self
            pinctrl.conf_nodes = [
                self.edt._node2enode[node] for node in prop.to_nodes()
            ]
            self.pinctrls.append(pinctrl)

        _add_names(node, "pinctrl", self.pinctrls)

    def _init_interrupts(self):
        # Initializes self.interrupts

        node = self._node

        self.interrupts = []

        for controller_node, data in _interrupts(node):
            interrupt = ControllerAndData()
            interrupt.node = self
            interrupt.controller = self.edt._node2enode[controller_node]
            interrupt.data = self._named_cells(interrupt.controller, data,
                                               "interrupt")

            self.interrupts.append(interrupt)

        _add_names(node, "interrupt", self.interrupts)

    def _standard_phandle_val_list(self, prop):
        # Parses a property like
        #
        #     <name>s = <phandle value phandle value ...>
        #     (e.g., pwms = <&foo 1 2 &bar 3 4>)
        #
        # , where each phandle points to a node that has a
        #
        #     #<name>-cells = <size>
        #
        # property that gives the number of cells in the value after the
        # phandle. These values are given names in *-cells in the binding for
        # the controller.
        #
        # Also parses any
        #
        #     <name>-names = "...", "...", ...
        #
        # Returns a list of Optional[ControllerAndData] instances.
        # An index is None if the underlying phandle-array element
        # is unspecified.

        if prop.name.endswith("gpios"):
            # There's some slight special-casing for *-gpios properties in that
            # e.g. foo-gpios still maps to #gpio-cells rather than
            # #foo-gpio-cells
            basename = "gpio"
        else:
            # Strip -s. We've already checked that the property names end in -s
            # in _check_prop_type_and_default().
            basename = prop.name[:-1]

        res = []

        for item in _phandle_val_list(prop, basename):
            if item is None:
                res.append(None)
                continue

            controller_node, data = item
            mapped_controller, mapped_data = \
                _map_phandle_array_entry(prop.node, controller_node, data,
                                         basename)

            entry = ControllerAndData()
            entry.node = self
            entry.controller = self.edt._node2enode[mapped_controller]
            entry.data = self._named_cells(entry.controller, mapped_data,
                                           basename)

            res.append(entry)

        _add_names(self._node, basename, res)

        return res

    def _named_cells(self, controller, data, basename):
        # Returns a dictionary that maps <basename>-cells names given in the
        # binding for 'controller' to cell values. 'data' is the raw data, as a
        # byte array.

        if not controller._binding:
            _err("{} controller {!r} for {!r} lacks binding"
                 .format(basename, controller._node, self._node))

        if basename in controller._binding.specifier2cells:
            cell_names = controller._binding.specifier2cells[basename]
        else:
            # Treat no *-cells in the binding the same as an empty *-cells, so
            # that bindings don't have to have e.g. an empty 'clock-cells:' for
            # '#clock-cells = <0>'.
            cell_names = []

        data_list = to_nums(data)
        if len(data_list) != len(cell_names):
            _err("unexpected '{}-cells:' length in binding for {!r} - {} "
                 "instead of {}"
                 .format(basename, controller._node, len(cell_names),
                         len(data_list)))

        return OrderedDict(zip(cell_names, data_list))


class Register:
    """
    Represents a register on a node.

    These attributes are available on Register objects:

    node:
      The Node instance this register is from

    name:
      The name of the register as given in the 'reg-names' property, or None if
      there is no 'reg-names' property

    addr:
      The starting address of the register, in the parent address space, or None
      if #address-cells is zero. Any 'ranges' properties are taken into account.

    size:
      The length of the register in bytes
    """
    def __repr__(self):
        fields = []

        if self.name is not None:
            fields.append("name: " + self.name)
        if self.addr is not None:
            fields.append("addr: " + hex(self.addr))
        if self.size is not None:
            fields.append("size: " + hex(self.size))

        return "<Register, {}>".format(", ".join(fields))


class ControllerAndData:
    """
    Represents an entry in an 'interrupts' or 'type: phandle-array' property
    value, e.g. <&ctrl-1 4 0> in

        cs-gpios = <&ctrl-1 4 0 &ctrl-2 3 4>;

    These attributes are available on ControllerAndData objects:

    node:
      The Node instance the property appears on

    controller:
      The Node instance for the controller (e.g. the controller the interrupt
      gets sent to for interrupts)

    data:
      A dictionary that maps names from the *-cells key in the binding for the
      controller to data values, e.g. {"pin": 4, "flags": 0} for the example
      above.

      'interrupts = <1 2>' might give {"irq": 1, "level": 2}.

    name:
      The name of the entry as given in
      'interrupt-names'/'gpio-names'/'pwm-names'/etc., or None if there is no
      *-names property
    """
    def __repr__(self):
        fields = []

        if self.name is not None:
            fields.append("name: " + self.name)

        fields.append("controller: {}".format(self.controller))
        fields.append("data: {}".format(self.data))

        return "<ControllerAndData, {}>".format(", ".join(fields))


class PinCtrl:
    """
    Represents a pin control configuration for a set of pins on a device,
    e.g. pinctrl-0 or pinctrl-1.

    These attributes are available on PinCtrl objects:

    node:
      The Node instance the pinctrl-* property is on

    name:
      The name of the configuration, as given in pinctrl-names, or None if
      there is no pinctrl-names property

    conf_nodes:
      A list of Node instances for the pin configuration nodes, e.g.
      the nodes pointed at by &state_1 and &state_2 in

          pinctrl-0 = <&state_1 &state_2>;
    """
    def __repr__(self):
        fields = []

        if self.name is not None:
            fields.append("name: " + self.name)

        fields.append("configuration nodes: " + str(self.conf_nodes))

        return "<PinCtrl, {}>".format(", ".join(fields))


class Property:
    """
    Represents a property on a Node, as set in its DT node and with
    additional info from the 'properties:' section of the binding.

    Only properties mentioned in 'properties:' get created. Properties of type
    'compound' currently do not get Property instances, as it's not clear
    what to generate for them.

    These attributes are available on Property objects. Several are
    just convenience accessors for attributes on the PropertySpec object
    accessible via the 'spec' attribute.

    These attributes are available on Property objects:

    node:
      The Node instance the property is on

    spec:
      The PropertySpec object which specifies this property.

    name:
      Convenience for spec.name.

    description:
      Convenience for spec.name with leading and trailing whitespace
      (including newlines) removed.

    type:
      Convenience for spec.type.

    val:
      The value of the property, with the format determined by spec.type,
      which comes from the 'type:' string in the binding.

        - For 'type: int/array/string/string-array', 'val' is what you'd expect
          (a Python integer or string, or a list of them)

        - For 'type: phandle' and 'type: path', 'val' is the pointed-to Node
          instance

        - For 'type: phandles', 'val' is a list of the pointed-to Node
          instances

        - For 'type: phandle-array', 'val' is a list of ControllerAndData
          instances. See the documentation for that class.

    val_as_token:
      The value of the property as a token, i.e. with non-alphanumeric
      characters replaced with underscores. This is only safe to access
      if self.enum_tokenizable returns True.

    enum_index:
      The index of 'val' in 'spec.enum' (which comes from the 'enum:' list
      in the binding), or None if spec.enum is None.
    """

    def __init__(self, spec, val, node):
        self.val = val
        self.spec = spec
        self.node = node

    @property
    def name(self):
        "See the class docstring"
        return self.spec.name

    @property
    def description(self):
        "See the class docstring"
        return self.spec.description.strip()

    @property
    def type(self):
        "See the class docstring"
        return self.spec.type

    @property
    def val_as_token(self):
        "See the class docstring"
        return re.sub(_NOT_ALPHANUM_OR_UNDERSCORE, '_', self.val)

    @property
    def enum_index(self):
        "See the class docstring"
        enum = self.spec.enum
        return enum.index(self.val) if enum else None

    def __repr__(self):
        fields = ["name: " + self.name,
                  # repr() to deal with lists
                  "type: " + self.type,
                  "value: " + repr(self.val)]

        if self.enum_index is not None:
            fields.append("enum index: {}".format(self.enum_index))

        return "<Property, {}>".format(", ".join(fields))


class Binding:
    """
    Represents a parsed binding.

    These attributes are available on Binding objects:

    path:
      The absolute path to the file defining the binding.

    description:
      The free-form description of the binding.

    compatible:
      The compatible string the binding matches.

      This may be None. For example, it's None when the Binding is inferred
      from node properties. It can also be None for Binding objects created
      using 'child-binding:' with no compatible.

    prop2specs:
      A collections.OrderedDict mapping property names to PropertySpec objects
      describing those properties' values.

    specifier2cells:
      A collections.OrderedDict that maps specifier space names (like "gpio",
      "clock", "pwm", etc.) to lists of cell names.

      For example, if the binding YAML contains 'pin' and 'flags' cell names
      for the 'gpio' specifier space, like this:

          gpio-cells:
          - pin
          - flags

      Then the Binding object will have a 'specifier2cells' attribute mapping
      "gpio" to ["pin", "flags"]. A missing key should be interpreted as zero
      cells.

    raw:
      The binding as an object parsed from YAML.

    bus:
      If nodes with this binding's 'compatible' describe a bus, a string
      describing the bus type (like "i2c"). None otherwise.

    on_bus:
      If nodes with this binding's 'compatible' appear on a bus, a string
      describing the bus type (like "i2c"). None otherwise.

    child_binding:
      If this binding describes the properties of child nodes, then
      this is a Binding object for those children; it is None otherwise.
      A Binding object's 'child_binding.child_binding' is not None if there
      are multiple levels of 'child-binding' descriptions in the binding.
    """

    def __init__(self, path, fname2path, raw=None,
                 require_compatible=True, require_description=True):
        """
        Binding constructor.

        path:
          Path to binding YAML file. May be None.

        fname2path:
          Map from include files to their absolute paths. Must
          not be None, but may be empty.

        raw:
          Optional raw content in the binding.
          This does not have to have any "include:" lines resolved.
          May be left out, in which case 'path' is opened and read.
          This can be used to resolve child bindings, for example.

        require_compatible:
          If True, it is an error if the binding does not contain a
          "compatible:" line. If False, a missing "compatible:" is
          not an error. Either way, "compatible:" must be a string
          if it is present in the binding.

        require_description:
          If True, it is an error if the binding does not contain a
          "description:" line. If False, a missing "description:" is
          not an error. Either way, "description:" must be a string
          if it is present in the binding.
        """
        self.path = path
        self._fname2path = fname2path

        if raw is None:
            with open(path, encoding="utf-8") as f:
                raw = yaml.load(f, Loader=_BindingLoader)

        # Merge any included files into self.raw. This also pulls in
        # inherited child binding definitions, so it has to be done
        # before initializing those.
        self.raw = self._merge_includes(raw, self.path)

        # Recursively initialize any child bindings. These don't
        # require a 'compatible' or 'description' to be well defined,
        # but they must be dicts.
        if "child-binding" in raw:
            if not isinstance(raw["child-binding"], dict):
                _err(f"malformed 'child-binding:' in {self.path}, "
                     "expected a binding (dictionary with keys/values)")
            self.child_binding = Binding(path, fname2path,
                                         raw=raw["child-binding"],
                                         require_compatible=False,
                                         require_description=False)
        else:
            self.child_binding = None

        # Make sure this is a well defined object.
        self._check(require_compatible, require_description)

        # Initialize look up tables.
        self.prop2specs = OrderedDict()
        for prop_name in self.raw.get("properties", {}).keys():
            self.prop2specs[prop_name] = PropertySpec(prop_name, self)
        self.specifier2cells = OrderedDict()
        for key, val in self.raw.items():
            if key.endswith("-cells"):
                self.specifier2cells[key[:-len("-cells")]] = val

    def __repr__(self):
        if self.compatible:
            compat = f" for compatible '{self.compatible}'"
        else:
            compat = ""
        return f"<Binding {os.path.basename(self.path)}" + compat + ">"

    @property
    def description(self):
        "See the class docstring"
        return self.raw['description']

    @property
    def compatible(self):
        "See the class docstring"
        return self.raw.get('compatible')

    @property
    def bus(self):
        "See the class docstring"
        return self.raw.get('bus')

    @property
    def on_bus(self):
        "See the class docstring"
        return self.raw.get('on-bus')

    def _merge_includes(self, raw, binding_path):
        # Constructor helper. Merges included files in
        # 'raw["include"]' into 'raw' using 'self._include_paths' as a
        # source of include files, removing the "include" key while
        # doing so.
        #
        # This treats 'binding_path' as the binding file being built up
        # and uses it for error messages.

        if "include" not in raw:
            return raw

        include = raw.pop("include")

        # First, merge the included files together. If more than one included
        # file has a 'required:' for a particular property, OR the values
        # together, so that 'required: true' wins.

        merged = {}

        if isinstance(include, str):
            # Simple scalar string case
            _merge_props(merged, self._load_raw(include), None, binding_path,
                         False)
        elif isinstance(include, list):
            # List of strings and maps. These types may be intermixed.
            for elem in include:
                if isinstance(elem, str):
                    _merge_props(merged, self._load_raw(elem), None,
                                 binding_path, False)
                elif isinstance(elem, dict):
                    name = elem.pop('name', None)
                    allowlist = elem.pop('property-allowlist', None)
                    blocklist = elem.pop('property-blocklist', None)
                    child_filter = elem.pop('child-binding', None)

                    if elem:
                        # We've popped out all the valid keys.
                        _err(f"'include:' in {binding_path} should not have "
                             f"these unexpected contents: {elem}")

                    _check_include_dict(name, allowlist, blocklist,
                                        child_filter, binding_path)

                    contents = self._load_raw(name)

                    _filter_properties(contents, allowlist, blocklist,
                                       child_filter, binding_path)
                    _merge_props(merged, contents, None, binding_path, False)
                else:
                    _err(f"all elements in 'include:' in {binding_path} "
                         "should be either strings or maps with a 'name' key "
                         "and optional 'property-allowlist' or "
                         f"'property-blocklist' keys, but got: {elem}")
        else:
            # Invalid item.
            _err(f"'include:' in {binding_path} "
                 f"should be a string or list, but has type {type(include)}")

        # Next, merge the merged included files into 'raw'. Error out if
        # 'raw' has 'required: false' while the merged included files have
        # 'required: true'.

        _merge_props(raw, merged, None, binding_path, check_required=True)

        return raw

    def _load_raw(self, fname):
        # Returns the contents of the binding given by 'fname' after merging
        # any bindings it lists in 'include:' into it. 'fname' is just the
        # basename of the file, so we check that there aren't multiple
        # candidates.

        path = self._fname2path.get(fname)

        if not path:
            _err(f"'{fname}' not found")

        with open(path, encoding="utf-8") as f:
            contents = yaml.load(f, Loader=_BindingLoader)

        return self._merge_includes(contents, path)

    def _check(self, require_compatible, require_description):
        # Does sanity checking on the binding.

        raw = self.raw

        if "compatible" in raw:
            compatible = raw["compatible"]
            if not isinstance(compatible, str):
                _err(f"malformed 'compatible: {compatible}' "
                     f"field in {self.path} - "
                     f"should be a string, not {type(compatible).__name__}")
        elif require_compatible:
            _err(f"missing 'compatible' in {self.path}")

        if "description" in raw:
            description = raw["description"]
            if not isinstance(description, str) or not description:
                _err(f"malformed or empty 'description' in {self.path}")
        elif require_description:
            _err(f"missing 'description' in {self.path}")

        # Allowed top-level keys. The 'include' key should have been
        # removed by _load_raw() already.
        ok_top = {"description", "compatible", "bus", "on-bus",
                  "properties", "child-binding"}

        # Descriptive errors for legacy bindings.
        legacy_errors = {
            "#cells": "expected *-cells syntax",
            "child": "use 'bus: <bus>' instead",
            "child-bus": "use 'bus: <bus>' instead",
            "parent": "use 'on-bus: <bus>' instead",
            "parent-bus": "use 'on-bus: <bus>' instead",
            "sub-node": "use 'child-binding' instead",
            "title": "use 'description' instead",
        }

        for key in raw:
            if key in legacy_errors:
                _err(f"legacy '{key}:' in {self.path}, {legacy_errors[key]}")

            if key not in ok_top and not key.endswith("-cells"):
                _err(f"unknown key '{key}' in {self.path}, "
                     "expected one of {', '.join(ok_top)}, or *-cells")

        for bus_key in "bus", "on-bus":
            if bus_key in raw and \
               not isinstance(raw[bus_key], str):
                _err(f"malformed '{bus_key}:' value in {self.path}, "
                     "expected string")

        self._check_properties()

        for key, val in raw.items():
            if key.endswith("-cells"):
                if not isinstance(val, list) or \
                   not all(isinstance(elem, str) for elem in val):
                    _err(f"malformed '{key}:' in {self.path}, "
                         "expected a list of strings")

    def _check_properties(self):
        # _check() helper for checking the contents of 'properties:'.

        raw = self.raw

        if "properties" not in raw:
            return

        ok_prop_keys = {"description", "type", "required",
                        "enum", "const", "default", "deprecated"}

        for prop_name, options in raw["properties"].items():
            for key in options:
                if key not in ok_prop_keys:
                    _err(f"unknown setting '{key}' in "
                         f"'properties: {prop_name}: ...' in {self.path}, "
                         f"expected one of {', '.join(ok_prop_keys)}")

            _check_prop_type_and_default(
                prop_name, options.get("type"),
                options.get("default"),
                self.path)

            for true_false_opt in ["required", "deprecated"]:
                if true_false_opt in options:
                    option = options[true_false_opt]
                    if not isinstance(option, bool):
                        _err(f"malformed '{true_false_opt}:' setting '{option}' "
                             f"for '{prop_name}' in 'properties' in {self.path}, "
                             "expected true/false")

            if options.get("deprecated") and options.get("required"):
                _err(f"'{prop_name}' in 'properties' in {self.path} should not "
                      "have both 'deprecated' and 'required' set")

            if "description" in options and \
               not isinstance(options["description"], str):
                _err("missing, malformed, or empty 'description' for "
                     f"'{prop_name}' in 'properties' in {self.path}")

            if "enum" in options and not isinstance(options["enum"], list):
                _err(f"enum in {self.path} for property '{prop_name}' "
                     "is not a list")

            if "const" in options and not isinstance(options["const"],
                                                     (int, str)):
                _err(f"const in {self.path} for property '{prop_name}' "
                     "is not a scalar")


def bindings_from_paths(yaml_paths, ignore_errors=False):
    """
    Get a list of Binding objects from the yaml files 'yaml_paths'.

    If 'ignore_errors' is True, YAML files that cause an EDTError when
    loaded are ignored. (No other exception types are silenced.)
    """

    ret = []
    fname2path = {os.path.basename(path): path for path in yaml_paths}
    for path in yaml_paths:
        try:
            ret.append(Binding(path, fname2path))
        except EDTError:
            if ignore_errors:
                continue
            raise

    return ret

class PropertySpec:
    """
    Represents a "property specification", i.e. the description of a
    property provided by a binding file, like its type and description.

    These attributes are available on PropertySpec objects:

    binding:
      The Binding object which defined this property.

    name:
      The property's name.

    path:
      The file where this property was defined. In case a binding includes
      other bindings, this is the file where the property was last modified.

    type:
      The type of the property as a string, as given in the binding.

    description:
      The free-form description of the property as a string, or None.

    enum:
      A list of values the property may take as given in the binding, or None.

    enum_tokenizable:
      True if enum is not None and all the values in it are tokenizable;
      False otherwise.

      A property must have string type and an "enum:" in its binding to be
      tokenizable. Additionally, the "enum:" values must be unique after
      converting all non-alphanumeric characters to underscores (so "foo bar"
      and "foo_bar" in the same "enum:" would not be tokenizable).

    enum_upper_tokenizable:
      Like 'enum_tokenizable', with the additional restriction that the
      "enum:" values must be unique after uppercasing and converting
      non-alphanumeric characters to underscores.

    const:
      The property's constant value as given in the binding, or None.

    default:
      The property's default value as given in the binding, or None.

    deprecated:
      True if the property is deprecated; False otherwise.

    required:
      True if the property is marked required; False otherwise.
    """

    def __init__(self, name, binding):
        self.binding = binding
        self.name = name
        self._raw = self.binding.raw["properties"][name]

    def __repr__(self):
        return f"<PropertySpec {self.name} type '{self.type}'>"

    @property
    def path(self):
        "See the class docstring"
        return self.binding.path

    @property
    def type(self):
        "See the class docstring"
        return self._raw["type"]

    @property
    def description(self):
        "See the class docstring"
        return self._raw.get("description")

    @property
    def enum(self):
        "See the class docstring"
        return self._raw.get("enum")

    @property
    def enum_tokenizable(self):
        "See the class docstring"
        if not hasattr(self, '_enum_tokenizable'):
            if self.type != 'string' or self.enum is None:
                self._enum_tokenizable = False
            else:
                # Saving _as_tokens here lets us reuse it in
                # enum_upper_tokenizable.
                self._as_tokens = [re.sub(_NOT_ALPHANUM_OR_UNDERSCORE,
                                          '_', value)
                                   for value in self.enum]
                self._enum_tokenizable = (len(self._as_tokens) ==
                                          len(set(self._as_tokens)))

        return self._enum_tokenizable

    @property
    def enum_upper_tokenizable(self):
        "See the class docstring"
        if not hasattr(self, '_enum_upper_tokenizable'):
            if not self.enum_tokenizable:
                self._enum_upper_tokenizable = False
            else:
                self._enum_upper_tokenizable = \
                    (len(self._as_tokens) ==
                     len(set(x.upper() for x in self._as_tokens)))
        return self._enum_upper_tokenizable

    @property
    def const(self):
        "See the class docstring"
        return self._raw.get("const")

    @property
    def default(self):
        "See the class docstring"
        return self._raw.get("default")

    @property
    def required(self):
        "See the class docstring"
        return self._raw.get("required", False)

    @property
    def deprecated(self):
        "See the class docstring"
        return self._raw.get("deprecated", False)

class EDTError(Exception):
    "Exception raised for devicetree- and binding-related errors"

#
# Public global functions
#


def load_vendor_prefixes_txt(vendor_prefixes):
    """Load a vendor-prefixes.txt file and return a dict
    representation mapping a vendor prefix to the vendor name.
    """
    vnd2vendor = {}
    with open(vendor_prefixes, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()

            if not line or line.startswith('#'):
                # Comment or empty line.
                continue

            # Other lines should be in this form:
            #
            # <vnd><TAB><vendor>
            vnd_vendor = line.split('\t', 1)
            assert len(vnd_vendor) == 2, line
            vnd2vendor[vnd_vendor[0]] = vnd_vendor[1]
    return vnd2vendor

#
# Private global functions
#


def _dt_compats(dt):
    # Returns a set() with all 'compatible' strings in the devicetree
    # represented by dt (a dtlib.DT instance)

    return {compat
            for node in dt.node_iter()
                if "compatible" in node.props
                    for compat in node.props["compatible"].to_strings()}


def _binding_paths(bindings_dirs):
    # Returns a list with the paths to all bindings (.yaml files) in
    # 'bindings_dirs'

    binding_paths = []

    for bindings_dir in bindings_dirs:
        for root, _, filenames in os.walk(bindings_dir):
            for filename in filenames:
                if filename.endswith(".yaml"):
                    binding_paths.append(os.path.join(root, filename))

    return binding_paths


def _binding_inc_error(msg):
    # Helper for reporting errors in the !include implementation

    raise yaml.constructor.ConstructorError(None, None, "error: " + msg)


def _check_include_dict(name, allowlist, blocklist, child_filter,
                        binding_path):
    # Check that an 'include:' named 'name' with property-allowlist
    # 'allowlist', property-blocklist 'blocklist', and
    # child-binding filter 'child_filter' has valid structure.

    if name is None:
        _err(f"'include:' element in {binding_path} "
             "should have a 'name' key")

    if allowlist is not None and blocklist is not None:
        _err(f"'include:' of file '{name}' in {binding_path} "
             "should not specify both 'property-allowlist:' "
             "and 'property-blocklist:'")

    while child_filter is not None:
        child_copy = deepcopy(child_filter)
        child_allowlist = child_copy.pop('property-allowlist', None)
        child_blocklist = child_copy.pop('property-blocklist', None)
        next_child_filter = child_copy.pop('child-binding', None)

        if child_copy:
            # We've popped out all the valid keys.
            _err(f"'include:' of file '{name}' in {binding_path} "
                 "should not have these unexpected contents in a "
                 f"'child-binding': {child_copy}")

        if child_allowlist is not None and child_blocklist is not None:
            _err(f"'include:' of file '{name}' in {binding_path} "
                 "should not specify both 'property-allowlist:' and "
                 "'property-blocklist:' in a 'child-binding:'")

        child_filter = next_child_filter


def _filter_properties(raw, allowlist, blocklist, child_filter,
                       binding_path):
    # Destructively modifies 'raw["properties"]' and
    # 'raw["child-binding"]', if they exist, according to
    # 'allowlist', 'blocklist', and 'child_filter'.

    props = raw.get('properties')
    _filter_properties_helper(props, allowlist, blocklist, binding_path)

    child_binding = raw.get('child-binding')
    while child_filter is not None and child_binding is not None:
        _filter_properties_helper(child_binding.get('properties'),
                                  child_filter.get('property-allowlist'),
                                  child_filter.get('property-blocklist'),
                                  binding_path)
        child_filter = child_filter.get('child-binding')
        child_binding = child_binding.get('child-binding')


def _filter_properties_helper(props, allowlist, blocklist, binding_path):
    if props is None or (allowlist is None and blocklist is None):
        return

    _check_prop_filter('property-allowlist', allowlist, binding_path)
    _check_prop_filter('property-blocklist', blocklist, binding_path)

    if allowlist is not None:
        allowset = set(allowlist)
        to_del = [prop for prop in props if prop not in allowset]
    else:
        blockset = set(blocklist)
        to_del = [prop for prop in props if prop in blockset]

    for prop in to_del:
        del props[prop]


def _check_prop_filter(name, value, binding_path):
    # Ensure an include: ... property-allowlist or property-blocklist
    # is a list.

    if value is None:
        return

    if not isinstance(value, list):
        _err(f"'{name}' value {value} in {binding_path} should be a list")


def _merge_props(to_dict, from_dict, parent, binding_path, check_required):
    # Recursively merges 'from_dict' into 'to_dict', to implement 'include:'.
    #
    # If 'from_dict' and 'to_dict' contain a 'required:' key for the same
    # property, then the values are ORed together.
    #
    # If 'check_required' is True, then an error is raised if 'from_dict' has
    # 'required: true' while 'to_dict' has 'required: false'. This prevents
    # bindings from "downgrading" requirements from bindings they include,
    # which might help keep bindings well-organized.
    #
    # It's an error for most other keys to appear in both 'from_dict' and
    # 'to_dict'. When it's not an error, the value in 'to_dict' takes
    # precedence.
    #
    # 'parent' is the name of the parent key containing 'to_dict' and
    # 'from_dict', and 'binding_path' is the path to the top-level binding.
    # These are used to generate errors for sketchy property overwrites.

    for prop in from_dict:
        if isinstance(to_dict.get(prop), dict) and \
           isinstance(from_dict[prop], dict):
            _merge_props(to_dict[prop], from_dict[prop], prop, binding_path,
                         check_required)
        elif prop not in to_dict:
            to_dict[prop] = from_dict[prop]
        elif _bad_overwrite(to_dict, from_dict, prop, check_required):
            _err("{} (in '{}'): '{}' from included file overwritten "
                 "('{}' replaced with '{}')".format(
                     binding_path, parent, prop, from_dict[prop],
                     to_dict[prop]))
        elif prop == "required":
            # Need a separate check here, because this code runs before
            # Binding._check()
            if not (isinstance(from_dict["required"], bool) and
                    isinstance(to_dict["required"], bool)):
                _err("malformed 'required:' setting for '{}' in 'properties' "
                     "in {}, expected true/false".format(parent, binding_path))

            # 'required: true' takes precedence
            to_dict["required"] = to_dict["required"] or from_dict["required"]


def _bad_overwrite(to_dict, from_dict, prop, check_required):
    # _merge_props() helper. Returns True in cases where it's bad that
    # to_dict[prop] takes precedence over from_dict[prop].

    if to_dict[prop] == from_dict[prop]:
        return False

    # These are overridden deliberately
    if prop in {"title", "description", "compatible"}:
        return False

    if prop == "required":
        if not check_required:
            return False
        return from_dict[prop] and not to_dict[prop]

    return True


def _binding_include(loader, node):
    # Implements !include, for backwards compatibility. '!include [foo, bar]'
    # just becomes [foo, bar].

    if isinstance(node, yaml.ScalarNode):
        # !include foo.yaml
        return [loader.construct_scalar(node)]

    if isinstance(node, yaml.SequenceNode):
        # !include [foo.yaml, bar.yaml]
        return loader.construct_sequence(node)

    _binding_inc_error("unrecognised node type in !include statement")


def _check_prop_type_and_default(prop_name, prop_type, default, binding_path):
    # Binding._check_properties() helper. Checks 'type:' and 'default:' for the
    # property named 'prop_name'

    if prop_type is None:
        _err("missing 'type:' for '{}' in 'properties' in {}"
             .format(prop_name, binding_path))

    ok_types = {"boolean", "int", "array", "uint8-array", "string",
                "string-array", "phandle", "phandles", "phandle-array",
                "path", "compound"}

    if prop_type not in ok_types:
        _err("'{}' in 'properties:' in {} has unknown type '{}', expected one "
             "of {}".format(prop_name, binding_path, prop_type,
                            ", ".join(ok_types)))

    if prop_type == "phandle-array" and not prop_name.endswith("s"):
        _err("'{}' in 'properties:' in {} is 'type: phandle-array', but its "
             "name does not end in -s. This is required since property names "
             "like '#pwm-cells' and 'pwm-names' get derived from 'pwms', for "
             "example.".format(prop_name, binding_path))

    # Check default

    if default is None:
        return

    if prop_type in {"boolean", "compound", "phandle", "phandles",
                     "phandle-array", "path"}:
        _err("'default:' can't be combined with 'type: {}' for '{}' in "
             "'properties:' in {}".format(prop_type, prop_name, binding_path))

    def ok_default():
        # Returns True if 'default' is an okay default for the property's type

        if prop_type == "int" and isinstance(default, int) or \
           prop_type == "string" and isinstance(default, str):
            return True

        # array, uint8-array, or string-array

        if not isinstance(default, list):
            return False

        if prop_type == "array" and \
           all(isinstance(val, int) for val in default):
            return True

        if prop_type == "uint8-array" and \
           all(isinstance(val, int) and 0 <= val <= 255 for val in default):
            return True

        # string-array
        return all(isinstance(val, str) for val in default)

    if not ok_default():
        _err("'default: {}' is invalid for '{}' in 'properties:' in {}, which "
             "has type {}".format(default, prop_name, binding_path, prop_type))


def _translate(addr, node):
    # Recursively translates 'addr' on 'node' to the address space(s) of its
    # parent(s), by looking at 'ranges' properties. Returns the translated
    # address.
    #
    # node:
    #   dtlib.Node instance

    if not node.parent or "ranges" not in node.parent.props:
        # No translation
        return addr

    if not node.parent.props["ranges"].value:
        # DT spec.: "If the property is defined with an <empty> value, it
        # specifies that the parent and child address space is identical, and
        # no address translation is required."
        #
        # Treat this the same as a 'range' that explicitly does a one-to-one
        # mapping, as opposed to there not being any translation.
        return _translate(addr, node.parent)

    # Gives the size of each component in a translation 3-tuple in 'ranges'
    child_address_cells = _address_cells(node)
    parent_address_cells = _address_cells(node.parent)
    child_size_cells = _size_cells(node)

    # Number of cells for one translation 3-tuple in 'ranges'
    entry_cells = child_address_cells + parent_address_cells + child_size_cells

    for raw_range in _slice(node.parent, "ranges", 4*entry_cells,
                            "4*(<#address-cells> (= {}) + "
                            "<#address-cells for parent> (= {}) + "
                            "<#size-cells> (= {}))"
                            .format(child_address_cells, parent_address_cells,
                                    child_size_cells)):
        child_addr = to_num(raw_range[:4*child_address_cells])
        raw_range = raw_range[4*child_address_cells:]

        parent_addr = to_num(raw_range[:4*parent_address_cells])
        raw_range = raw_range[4*parent_address_cells:]

        child_len = to_num(raw_range)

        if child_addr <= addr < child_addr + child_len:
            # 'addr' is within range of a translation in 'ranges'. Recursively
            # translate it and return the result.
            return _translate(parent_addr + addr - child_addr, node.parent)

    # 'addr' is not within range of any translation in 'ranges'
    return addr


def _add_names(node, names_ident, objs):
    # Helper for registering names from <foo>-names properties.
    #
    # node:
    #   edtlib.Node instance
    #
    # names-ident:
    #   The <foo> part of <foo>-names, e.g. "reg" for "reg-names"
    #
    # objs:
    #   list of objects whose .name field should be set

    full_names_ident = names_ident + "-names"

    if full_names_ident in node.props:
        names = node.props[full_names_ident].to_strings()
        if len(names) != len(objs):
            _err("{} property in {} in {} has {} strings, expected {} strings"
                 .format(full_names_ident, node.path, node.dt.filename,
                         len(names), len(objs)))

        for obj, name in zip(objs, names):
            if obj is None:
                continue
            obj.name = name
    else:
        for obj in objs:
            if obj is not None:
                obj.name = None


def _interrupt_parent(node):
    # Returns the node pointed at by the closest 'interrupt-parent', searching
    # the parents of 'node'. As of writing, this behavior isn't specified in
    # the DT spec., but seems to match what some .dts files except.

    while node:
        if "interrupt-parent" in node.props:
            return node.props["interrupt-parent"].to_node()
        node = node.parent

    _err("{!r} has an 'interrupts' property, but neither the node nor any "
         "of its parents has an 'interrupt-parent' property".format(node))


def _interrupts(node):
    # Returns a list of (<controller>, <data>) tuples, with one tuple per
    # interrupt generated by 'node'. <controller> is the destination of the
    # interrupt (possibly after mapping through an 'interrupt-map'), and <data>
    # the data associated with the interrupt (as a 'bytes' object).

    # Takes precedence over 'interrupts' if both are present
    if "interrupts-extended" in node.props:
        prop = node.props["interrupts-extended"]
        return [_map_interrupt(node, iparent, spec)
                for iparent, spec in _phandle_val_list(prop, "interrupt")]

    if "interrupts" in node.props:
        # Treat 'interrupts' as a special case of 'interrupts-extended', with
        # the same interrupt parent for all interrupts

        iparent = _interrupt_parent(node)
        interrupt_cells = _interrupt_cells(iparent)

        return [_map_interrupt(node, iparent, raw)
                for raw in _slice(node, "interrupts", 4*interrupt_cells,
                                  "4*<#interrupt-cells>")]

    return []


def _map_interrupt(child, parent, child_spec):
    # Translates an interrupt headed from 'child' to 'parent' with data
    # 'child_spec' through any 'interrupt-map' properties. Returns a
    # (<controller>, <data>) tuple with the final destination after mapping.

    if "interrupt-controller" in parent.props:
        return (parent, child_spec)

    def own_address_cells(node):
        # Used for parents pointed at by 'interrupt-map'. We can't use
        # _address_cells(), because it's the #address-cells property on 'node'
        # itself that matters.

        address_cells = node.props.get("#address-cells")
        if not address_cells:
            _err("missing #address-cells on {!r} (while handling interrupt-map)"
                 .format(node))
        return address_cells.to_num()

    def spec_len_fn(node):
        # Can't use _address_cells() here, because it's the #address-cells
        # property on 'node' itself that matters
        return own_address_cells(node) + _interrupt_cells(node)

    parent, raw_spec = _map(
        "interrupt", child, parent, _raw_unit_addr(child) + child_spec,
        spec_len_fn, require_controller=True)

    # Strip the parent unit address part, if any
    return (parent, raw_spec[4*own_address_cells(parent):])


def _map_phandle_array_entry(child, parent, child_spec, basename):
    # Returns a (<controller>, <data>) tuple with the final destination after
    # mapping through any '<basename>-map' (e.g. gpio-map) properties. See
    # _map_interrupt().

    def spec_len_fn(node):
        prop_name = "#{}-cells".format(basename)
        if prop_name not in node.props:
            _err("expected '{}' property on {!r} (referenced by {!r})"
                 .format(prop_name, node, child))
        return node.props[prop_name].to_num()

    # Do not require <prefix>-controller for anything but interrupts for now
    return _map(basename, child, parent, child_spec, spec_len_fn,
                require_controller=False)


def _map(prefix, child, parent, child_spec, spec_len_fn, require_controller):
    # Common code for mapping through <prefix>-map properties, e.g.
    # interrupt-map and gpio-map.
    #
    # prefix:
    #   The prefix, e.g. "interrupt" or "gpio"
    #
    # child:
    #   The "sender", e.g. the node with 'interrupts = <...>'
    #
    # parent:
    #   The "receiver", e.g. a node with 'interrupt-map = <...>' or
    #   'interrupt-controller' (no mapping)
    #
    # child_spec:
    #   The data associated with the interrupt/GPIO/etc., as a 'bytes' object,
    #   e.g. <1 2> for 'foo-gpios = <&gpio1 1 2>'.
    #
    # spec_len_fn:
    #   Function called on a parent specified in a *-map property to get the
    #   length of the parent specifier (data after phandle in *-map), in cells
    #
    # require_controller:
    #   If True, the final controller node after mapping is required to have
    #   to have a <prefix>-controller property.

    map_prop = parent.props.get(prefix + "-map")
    if not map_prop:
        if require_controller and prefix + "-controller" not in parent.props:
            _err("expected '{}-controller' property on {!r} "
                 "(referenced by {!r})".format(prefix, parent, child))

        # No mapping
        return (parent, child_spec)

    masked_child_spec = _mask(prefix, child, parent, child_spec)

    raw = map_prop.value
    while raw:
        if len(raw) < len(child_spec):
            _err("bad value for {!r}, missing/truncated child data"
                 .format(map_prop))
        child_spec_entry = raw[:len(child_spec)]
        raw = raw[len(child_spec):]

        if len(raw) < 4:
            _err("bad value for {!r}, missing/truncated phandle"
                 .format(map_prop))
        phandle = to_num(raw[:4])
        raw = raw[4:]

        # Parent specified in *-map
        map_parent = parent.dt.phandle2node.get(phandle)
        if not map_parent:
            _err("bad phandle ({}) in {!r}".format(phandle, map_prop))

        map_parent_spec_len = 4*spec_len_fn(map_parent)
        if len(raw) < map_parent_spec_len:
            _err("bad value for {!r}, missing/truncated parent data"
                 .format(map_prop))
        parent_spec = raw[:map_parent_spec_len]
        raw = raw[map_parent_spec_len:]

        # Got one *-map row. Check if it matches the child data.
        if child_spec_entry == masked_child_spec:
            # Handle *-map-pass-thru
            parent_spec = _pass_thru(
                prefix, child, parent, child_spec, parent_spec)

            # Found match. Recursively map and return it.
            return _map(prefix, parent, map_parent, parent_spec, spec_len_fn,
                        require_controller)

    _err("child specifier for {!r} ({}) does not appear in {!r}"
         .format(child, child_spec, map_prop))


def _mask(prefix, child, parent, child_spec):
    # Common code for handling <prefix>-mask properties, e.g. interrupt-mask.
    # See _map() for the parameters.

    mask_prop = parent.props.get(prefix + "-map-mask")
    if not mask_prop:
        # No mask
        return child_spec

    mask = mask_prop.value
    if len(mask) != len(child_spec):
        _err("{!r}: expected '{}-mask' in {!r} to be {} bytes, is {} bytes"
             .format(child, prefix, parent, len(child_spec), len(mask)))

    return _and(child_spec, mask)


def _pass_thru(prefix, child, parent, child_spec, parent_spec):
    # Common code for handling <prefix>-map-thru properties, e.g.
    # interrupt-pass-thru.
    #
    # parent_spec:
    #   The parent data from the matched entry in the <prefix>-map property
    #
    # See _map() for the other parameters.

    pass_thru_prop = parent.props.get(prefix + "-map-pass-thru")
    if not pass_thru_prop:
        # No pass-thru
        return parent_spec

    pass_thru = pass_thru_prop.value
    if len(pass_thru) != len(child_spec):
        _err("{!r}: expected '{}-map-pass-thru' in {!r} to be {} bytes, is {} bytes"
             .format(child, prefix, parent, len(child_spec), len(pass_thru)))

    res = _or(_and(child_spec, pass_thru),
              _and(parent_spec, _not(pass_thru)))

    # Truncate to length of parent spec.
    return res[-len(parent_spec):]


def _raw_unit_addr(node):
    # _map_interrupt() helper. Returns the unit address (derived from 'reg' and
    # #address-cells) as a raw 'bytes'

    if 'reg' not in node.props:
        _err("{!r} lacks 'reg' property (needed for 'interrupt-map' unit "
             "address lookup)".format(node))

    addr_len = 4*_address_cells(node)

    if len(node.props['reg'].value) < addr_len:
        _err("{!r} has too short 'reg' property (while doing 'interrupt-map' "
             "unit address lookup)".format(node))

    return node.props['reg'].value[:addr_len]


def _and(b1, b2):
    # Returns the bitwise AND of the two 'bytes' objects b1 and b2. Pads
    # with ones on the left if the lengths are not equal.

    # Pad on the left, to equal length
    maxlen = max(len(b1), len(b2))
    return bytes(x & y for x, y in zip(b1.rjust(maxlen, b'\xff'),
                                       b2.rjust(maxlen, b'\xff')))


def _or(b1, b2):
    # Returns the bitwise OR of the two 'bytes' objects b1 and b2. Pads with
    # zeros on the left if the lengths are not equal.

    # Pad on the left, to equal length
    maxlen = max(len(b1), len(b2))
    return bytes(x | y for x, y in zip(b1.rjust(maxlen, b'\x00'),
                                       b2.rjust(maxlen, b'\x00')))


def _not(b):
    # Returns the bitwise not of the 'bytes' object 'b'

    # ANDing with 0xFF avoids negative numbers
    return bytes(~x & 0xFF for x in b)


def _phandle_val_list(prop, n_cells_name):
    # Parses a '<phandle> <value> <phandle> <value> ...' value. The number of
    # cells that make up each <value> is derived from the node pointed at by
    # the preceding <phandle>.
    #
    # prop:
    #   dtlib.Property with value to parse
    #
    # n_cells_name:
    #   The <name> part of the #<name>-cells property to look for on the nodes
    #   the phandles point to, e.g. "gpio" for #gpio-cells.
    #
    # Returns a list[Optional[tuple]].
    #
    # Each tuple in the list is a (<node>, <value>) pair, where <node>
    # is the node pointed at by <phandle>. If <phandle> does not refer
    # to a node, the entire list element is None.

    full_n_cells_name = "#{}-cells".format(n_cells_name)

    res = []

    raw = prop.value
    while raw:
        if len(raw) < 4:
            # Not enough room for phandle
            _err("bad value for " + repr(prop))
        phandle = to_num(raw[:4])
        raw = raw[4:]

        node = prop.node.dt.phandle2node.get(phandle)
        if not node:
            # Unspecified phandle-array element. This is valid; a 0
            # phandle value followed by no cells is an empty element.
            res.append(None)
            continue

        if full_n_cells_name not in node.props:
            _err("{!r} lacks {}".format(node, full_n_cells_name))

        n_cells = node.props[full_n_cells_name].to_num()
        if len(raw) < 4*n_cells:
            _err("missing data after phandle in " + repr(prop))

        res.append((node, raw[:4*n_cells]))
        raw = raw[4*n_cells:]

    return res


def _address_cells(node):
    # Returns the #address-cells setting for 'node', giving the number of <u32>
    # cells used to encode the address in the 'reg' property

    if "#address-cells" in node.parent.props:
        return node.parent.props["#address-cells"].to_num()
    return 2  # Default value per DT spec.


def _size_cells(node):
    # Returns the #size-cells setting for 'node', giving the number of <u32>
    # cells used to encode the size in the 'reg' property

    if "#size-cells" in node.parent.props:
        return node.parent.props["#size-cells"].to_num()
    return 1  # Default value per DT spec.


def _interrupt_cells(node):
    # Returns the #interrupt-cells property value on 'node', erroring out if
    # 'node' has no #interrupt-cells property

    if "#interrupt-cells" not in node.props:
        _err("{!r} lacks #interrupt-cells".format(node))
    return node.props["#interrupt-cells"].to_num()


def _slice(node, prop_name, size, size_hint):
    # Splits node.props[prop_name].value into 'size'-sized chunks, returning a
    # list of chunks. Raises EDTError if the length of the property is not
    # evenly divisible by 'size'. 'size_hint' is a string shown on errors that
    # gives a hint on how 'size' was calculated.

    raw = node.props[prop_name].value
    if len(raw) % size:
        _err("'{}' property in {!r} has length {}, which is not evenly "
             "divisible by {} (= {}). Note that #*-cells "
             "properties come either from the parent node or from the "
             "controller (in the case of 'interrupts')."
             .format(prop_name, node, len(raw), size, size_hint))

    return [raw[i:i + size] for i in range(0, len(raw), size)]


def _check_dt(dt):
    # Does devicetree sanity checks. dtlib is meant to be general and
    # anything-goes except for very special properties like phandle, but in
    # edtlib we can be pickier.

    # Check that 'status' has one of the values given in the devicetree spec.

    # Accept "ok" for backwards compatibility
    ok_status = {"ok", "okay", "disabled", "reserved", "fail", "fail-sss"}

    for node in dt.node_iter():
        if "status" in node.props:
            try:
                status_val = node.props["status"].to_string()
            except DTError as e:
                # The error message gives the path
                _err(str(e))

            if status_val not in ok_status:
                _err("unknown 'status' value \"{}\" in {} in {}, expected one "
                     "of {} (see the devicetree specification)"
                     .format(status_val, node.path, node.dt.filename,
                             ", ".join(ok_status)))

        ranges_prop = node.props.get("ranges")
        if ranges_prop:
            if ranges_prop.type not in (Type.EMPTY, Type.NUMS):
                _err("expected 'ranges = < ... >;' in {} in {}, not '{}' "
                     "(see the devicetree specification)"
                     .format(node.path, node.dt.filename, ranges_prop))


def _err(msg):
    raise EDTError(msg)

# Logging object
_LOG = logging.getLogger(__name__)

# Regular expression for non-alphanumeric-or-underscore characters.
_NOT_ALPHANUM_OR_UNDERSCORE = re.compile(r'\W', re.ASCII)

# Custom PyYAML binding loader class to avoid modifying yaml.Loader directly,
# which could interfere with YAML loading in clients
class _BindingLoader(Loader):
    pass


# Add legacy '!include foo.yaml' handling
_BindingLoader.add_constructor("!include", _binding_include)

# Use OrderedDict instead of plain dict for YAML mappings, to preserve
# insertion order on Python 3.5 and earlier (plain dicts only preserve
# insertion order on Python 3.6+). This makes testing easier and avoids
# surprises.
#
# Adapted from
# https://stackoverflow.com/questions/5121931/in-python-how-can-you-load-yaml-mappings-as-ordereddicts.
# Hopefully this API stays stable.
_BindingLoader.add_constructor(
    yaml.resolver.BaseResolver.DEFAULT_MAPPING_TAG,
    lambda loader, node: OrderedDict(loader.construct_pairs(node)))

#
# "Default" binding for properties which are defined by the spec.
#
# Zephyr: do not change the _DEFAULT_PROP_TYPES keys without
# updating the documentation for the DT_PROP() macro in
# include/devicetree.h.
#

_DEFAULT_PROP_TYPES = {
    "compatible": "string-array",
    "status": "string",
    "reg": "array",
    "reg-names": "string-array",
    "label": "string",
    "interrupts": "array",
    "interrupts-extended": "compound",
    "interrupt-names": "string-array",
    "interrupt-controller": "boolean",
}

_STATUS_ENUM = "ok okay disabled reserved fail fail-sss".split()

def _raw_default_property_for(name):
    ret = {
        'type': _DEFAULT_PROP_TYPES[name],
        'required': False,
    }
    if name == 'status':
        ret['enum'] = _STATUS_ENUM
    return ret

_DEFAULT_PROP_BINDING = Binding(
    None, {},
    raw={
        'properties': {
            name: _raw_default_property_for(name)
            for name in _DEFAULT_PROP_TYPES
        },
    },
    require_compatible=False, require_description=False,
)

_DEFAULT_PROP_SPECS = {
    name: PropertySpec(name, _DEFAULT_PROP_BINDING)
    for name in _DEFAULT_PROP_TYPES
}

# A set of vendor prefixes which are grandfathered in by Linux,
# and therefore by us as well.
_VENDOR_PREFIX_ALLOWED = set([
    "at25", "bm", "devbus", "dmacap", "dsa",
    "exynos", "fsia", "fsib", "gpio-fan", "gpio-key", "gpio", "gpmc",
    "hdmi", "i2c-gpio", "keypad", "m25p", "max8952", "max8997",
    "max8998", "mpmc", "pinctrl-single", "#pinctrl-single", "PowerPC",
    "pl022", "pxa-mmc", "rcar_sound", "rotary-encoder", "s5m8767",
    "sdhci", "simple-audio-card", "st-plgpio", "st-spics", "ts",
])
