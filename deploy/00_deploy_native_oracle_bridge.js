module.exports = async ({getNamedAccounts, deployments}) => {
    const {deploy} = deployments;
    const {deployer} = await getNamedAccounts();


    const bridge = await deploy('NativeOracleBridge', {
        from: deployer,
        args: ["1000000000000000000", 10],
    });

    console.log("Deployed to:", bridge.address);
    const tester = await deploy('NativeOracleBridgeTester', {
        from: deployer,
        args: [bridge.address],
    });
    console.log("Deployed to:", tester.address);

};
module.exports.tags = ['NativeOracleBridge'];